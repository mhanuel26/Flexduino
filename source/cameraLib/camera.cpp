/*
 * Copyright (c) 2016, Manuel Iglesias
 * All rights reserved.
 *
 *
 *
 */


#include "camera.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "FatFs.h"			// out FatFs c++ library based on FatFs from elm ChaN
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Serial.h"
#include "Arduino.h"

extern "C" {
#include "flexio_ov7670.h"
// .jpeg encoder
#include "jpegenc.h"
#include "image_defs.h"
}
#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "fsl_port.h"
#include "fsl_pit.h"
// websockets support
#include "WebSockets.h"
#include "WebSocketsServer.h"

#include <assert.h>

#define DEBUG_CAM	1
#define DEBUG_YUV	0

/*******************************************************************************
 * Definitions
 ******************************************************************************/

// constants specific to 640 x 480 image
#define IMG_WIDTH   (160)   //
#define IMG_HEIGHT  (120)   //
#define NUM_LINES   (15)    // number of lines in image as X*16/2=120 => X = 15

/*******************************************************************************
 * Constants
 ******************************************************************************/

const unsigned char bmp_header[BMPIMAGEOFFSET] =
{
      0x42, 0x4D, 0x42, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00,
      0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
      0x00, 0x00, 0x00, 0x96, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
      0x00, 0x00
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t __attribute__((aligned(32), section(".myRAM"))) imgBuffer[10000];			// don't know the precise size so it's better to oversize it...
																						// nxp you rock but please add memory to this monster...
videoType webCam::video_type_ = no_video;
img_fmt	webCam::format_ = yuv422;
int webCam::file_jpg_ = -1;
volatile int8_t webCam::cur_flexio_macroblk_ = -1;
int8_t webCam::cur_stack_macroblk_ = -1;
int8_t webCam::wsnum_ = -1;
uint8_t* webCam::_imgPtr = &imgBuffer[0];
uint16_t webCam::_imgSize = 0;

volatile __attribute__((aligned(32), section(".myRAM"))) uint8_t g_FlexioCameraMacroblockBuffer[5][MACROBLOCK];
//extern volatile uint8_t g_FlexioCameraFrameBuffer[2][OV7670_FRAME_BYTES + 32];


extern FileFs file;
extern SerialConsole Serial;
extern boolean has_filesystem;
extern WebSocketsServer webSocket;

/*******************************************************************************
 * Code
 ******************************************************************************/

// wrapper global function  for writing output encoded .jpeg data
extern "C" void* C_webCam(){ return (void*)&camera;}

extern "C" void C_write_jpeg(void *webCamClass, const unsigned char * _buffer, const unsigned int _n)
{
	return static_cast<webCam*>(webCamClass)->write_jpeg(_buffer, _n);
}

static void webCamJpegMacroBlkCb(uint8_t buf){
	camera.jpegMacroBlkCb(buf);
}

webCam::webCam(){
	registerCamCallback(webCamJpegMacroBlkCb);
}

void webCam::begin(){
	Serial.println("calling camera init");
	gpio_pin_config_t pinConfig;

    /* enable OV7670 Reset output */
    pinConfig.pinDirection = kGPIO_DigitalOutput;
    pinConfig.outputLogic = 1U;
    GPIO_PinInit(GPIOC, 8U, &pinConfig);


    memset((void*)g_FlexioCameraMacroblockBuffer, 0, sizeof(g_FlexioCameraMacroblockBuffer));
    FLEXIO_Ov7670Init();

    // just to debug arduino Style
//    Serial.print("SIM->SOPT2 = 0b");
//    Serial.println((uint32_t)SIM->SOPT2, BIN);

}

volatile void webCam::jpegMacroBlkCb(uint8_t buf){
	cur_flexio_macroblk_ = buf;
}

void webCam::registerSingleCaptureCb( imageSingleCaptureCb fnctCb ){
	captureCb = fnctCb;
}

void webCam::picturemode(void){
	video_type_ = single_capture;
}

void webCam::picturemode(saveMemory memory){
	if(memory == RAM){
		video_type_ = single_capture;
	}else{
		video_type_ = single_2sdcard;
	}
}

void webCam::startvideo(uint8_t wsnum){
	// this a simple wrapper preparing for the advance hack...now let's play...
	wsnum_ = wsnum;
	shot_done = false;
	cur_stack_macroblk_ = -1;
	cur_flexio_macroblk_ = -1;
	video_type_ = bitbang;
	FLEXIO_Ov7670StartCapture( 1 );
}

void webCam::stopvideo(){
	video_type_ = no_video;
	wsnum_ = -1; 		// not valid id for websocket
	FLEXIO_Ov7670StartCapture( 0 );			// force stop
}

videoType webCam::videomode(){
	return video_type_;
}

void webCam::snapshot(){
#if DEBUG_CAM
	Serial.println("start image capture command");
#endif
	shot_done = false;
	_imgSize = 0;
	FLEXIO_Ov7670StartCapture( 1 );
}

bool webCam::shotdone(){
	return shot_done;
}

uint16_t webCam::getImgSize(void){
	return _imgSize;
}

void webCam::process(){
	if(cur_stack_macroblk_ < cur_flexio_macroblk_){
		if(cur_stack_macroblk_ < 0){
			cur_stack_macroblk_ = 0;
		}else{
			cur_stack_macroblk_++;
		}
//		assert(!((cur_stack_macroblk_ >= MACROLINES) || (cur_stack_macroblk_ < 0)));
		uint8_t *dat = (uint8_t *)&g_FlexioCameraMacroblockBuffer[cur_stack_macroblk_][0];
		compress_macroblock(dat);
	}
}


void webCam::write_jpeg(const unsigned char * _buffer, const unsigned int _n){
	switch(video_type_){
		case bitbang:
			webSocket.sendBIN(wsnum_, (const uint8_t*)_buffer, _n);
		break;
		case single_2sdcard:
		case single_capture:
			// here we are setting a capture to a RAM buffer
			memcpy(_imgPtr, _buffer, _n); 		// why the f.. underscore.
			_imgPtr += _n;			// increase the pointer
		break;
		default:
			//not implemented or do not need any processing here
		break;
	}
}

// this function will jpeg encode a macroblock, normally n number of 16 by 16 YUV microblocks, where n is given by IMAGE_WIDTH/16
bool webCam::compress_macroblock(uint8_t *buffer){
	if(cur_stack_macroblk_ == 0){
		if(video_type_ == no_video){
#if DEBUG_CAM
			Serial.println("Open JPG file");
#endif
			openJpeg();
		}
	    // write .jpeg header and start-of-image marker
	    huffman_start(IMG_HEIGHT & -8, IMG_WIDTH & -8);
	    huffman_resetdc();
	}
	encode_line_yuv(buffer,  cur_stack_macroblk_);
	if(cur_stack_macroblk_ >= (MACROLINES - 1)){
		// set internals
		shot_done = true;
		cur_stack_macroblk_ = -1;
		cur_flexio_macroblk_ = -1;
		// write .jpeg footer (end-of-image marker)
		huffman_stop();
		int fh;
		switch(video_type_){
			case bitbang:
				// send end command to javascript worker
				webSocket.sendTXT(wsnum_, "end");
			break;
			case single_capture:
				// do something with image in buffer
				// or just let the upper layer to poll shot_done variable and take action
				// not before setting buffer size
				_imgSize = _imgPtr - &imgBuffer[0];
				// If there is a callback defined
#if DEBUG_CAM
				Serial.println("JPG image capture done");
#endif
				if(captureCb)
					captureCb();
			break;
			case single_2sdcard:
				// set the buffer size
				_imgSize = _imgPtr - &imgBuffer[0];
				//  this nmeed abstraction, we are mixing something from Audio Playback here...
#if PLAYBACK_EN
				PIT_DisableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
#endif
				// save the jpeg image to SD card
				fh = file.open("single_capture.jpg", FA_WRITE | FA_CREATE_ALWAYS);
				if(fh < 0)
					return false;
				file.write(fh, (void*)&imgBuffer[0], _imgSize);
				file.close(fh);
#if PLAYBACK_EN
				// again audio...
				PIT_EnableInterrupts(PIT, kPIT_Chnl_2, kPIT_TimerInterruptEnable);
#endif
#if DEBUG_CAM
				Serial.println("JPG file saved under single capture mode");
#endif
			break;
			default:
				//not implemented
			break;
		}
		return true;
	}
	return false;
}

// The following functions are not used in our mode of conerting to jpeg each macroblock..


void webCam::openJpeg(void){
	// this was not needed anymore...
}

void webCam::closeJpeg(void){
    // write .jpeg footer (end-of-image marker)
    huffman_stop();
    switch(video_type_){
		case bitbang:
			// send end command to javascript worker
			webSocket.sendTXT(wsnum_, "end");
		break;
		case single_capture:

		break;
		default:
			//not implemented
		break;
    }
}


// this function will jpeg encode a whole frame at once blocking, also needs the whole frame in buffer.
void webCam::compress(uint8_t *buffer){
#if DEBUG_CAM
	uint32_t start = millis();
#endif
    // line buffer
    uint8_t *line_buffer_ptr = buffer;
    openJpeg();
    // write .jpeg header and start-of-image marker
    huffman_start(IMG_HEIGHT & -8, IMG_WIDTH & -8);
    huffman_resetdc();

    // try to load data from file one line at a time
    // number of lines in image (specific to 160 x 120 image and encoding rate)
    uint16_t line;

    for (line=0; line<NUM_LINES; line++) {
        // load line OV7670 buffer
		encode_line_yuv(line_buffer_ptr,  line);
		line_buffer_ptr += 2560;
    }
    closeJpeg();
#if DEBUG_CAM
    Serial.print("compresed YUV to jpeg took ");
    Serial.print(millis()-start);
    Serial.println(" ms");
#endif
    return;
}


webCam camera;

