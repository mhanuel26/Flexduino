#ifndef MM_IOT_H
#define MM_IOT_H

void sendImageCb(void);

class mm_iot{
public:
	void uploadImageUrlDoneCb(void);
	int loadImageInJson(void);
	void parseImgurUpload(char *json);
	void sendImgurImageCb(void);

private:
	char _id[10];
};

mm_iot	mmIoT;

#endif
