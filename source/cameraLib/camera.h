#ifndef __VIDEO_WEB_CAM_H__
#define __VIDEO_WEB_CAM_H__

#include <stdint.h>

#define BMPIMAGEOFFSET 66

typedef enum videoType{
	no_video=0,
	bitbang,
	FlexInterleaved					// this will be the advance mode, where we are going to compress 16 MCU's after received by FlexIO
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
	bool shotdone();
	volatile void jpegMacroBlkCb(uint8_t buf);
	volatile void pingPongManager(uint8_t buf);
	void write_jpeg(const unsigned char * _buffer, const unsigned int _n);
	void compress(uint8_t *buffer);
	bool compress_macroblock(uint8_t *buffer);
	void openJpeg(void);
	void closeJpeg(void);

private:
	static videoType video_type_;
	static img_fmt format_;
	static int file_jpg_;
	static volatile bool video_mode_;
	static volatile int8_t cur_flexio_macroblk_;
	static int8_t cur_stack_macroblk_;

public:
	bool shot_done;
	static int8_t wsnum_;
};

extern webCam camera;

#endif
