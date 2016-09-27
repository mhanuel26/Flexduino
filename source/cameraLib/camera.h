#ifndef __VIDEO_WEB_CAM_H__
#define __VIDEO_WEB_CAM_H__

#include <stdint.h>

#define BMPIMAGEOFFSET 66

typedef void (*imageSingleCaptureCb)( void );

typedef enum saveMemory{
	RAM,
	SDcard
}saveMemory;

typedef enum videoType{
	no_video=0,
	bitbang,
	single_capture,				// this mode we are going to compress and keep the image in RAM buffer for speed but also since we will discard it when the next one come
	single_2sdcard				// same as before but save it to sd card at the end
}videoType;

typedef enum img_fmt{
	raw,
	yuv422,
	rgb565,
	rgb555
}img_fmt;

class webCam {
public:
	webCam();
	void begin(void);
	void process(void);
	void snapshot();
	void startvideo(uint8_t wsnum);
	void stopvideo();
	videoType videomode();
	void picturemode(void);
	void picturemode(saveMemory memory);
	bool shotdone();
	volatile void jpegMacroBlkCb(uint8_t buf);
	volatile void pingPongManager(uint8_t buf);
	void write_jpeg(const unsigned char * _buffer, const unsigned int _n);
	uint16_t getImgSize(void);
	void compress(uint8_t *buffer);
	bool compress_macroblock(uint8_t *buffer);
	void openJpeg(void);
	void closeJpeg(void);
	void registerSingleCaptureCb( imageSingleCaptureCb fnctCb );
private:
	static videoType video_type_;
	static img_fmt format_;
	static int file_jpg_;
	static volatile int8_t cur_flexio_macroblk_;
	static int8_t cur_stack_macroblk_;
	static uint8_t *_imgPtr;
	static uint16_t _imgSize;
	imageSingleCaptureCb	captureCb;
public:
	bool shot_done;
	static int8_t wsnum_;
};

extern webCam camera;

#endif
