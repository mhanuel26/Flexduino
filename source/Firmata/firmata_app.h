#ifndef firmata_app_h
#define firmata_app_h

// Firmata Library
#if defined(WINC1500)
#define WIFI_101
#endif

#define D0		0b00000001
#define D1		0b00000010
#define D2		0b00000100
#define D3		0b00001000
#define D4		0b00010000
#define	D5		0b00100000
#define	D6		0b01000000

#define nD0		0
#define nD1		1
#define nD2		2
#define nD3		3
#define nD4		4
#define nD5		5
#define nD6		6
#define nD7		7

#define SERVER_PORT		3030

#define OFF	false
#define ON	true

#include "Firmata.h"

extern char ssid[];
extern char pass[];
extern void printWifiStatus(void);

extern "C" {
// callback function types
typedef void (*appCallbackFunction)(byte, bool);

typedef void (*initCallbackFunction)(void);

}


class espNode {
public:
	espNode(){ };
	void espNodeInit();
	void appSetDigitalPinCb(byte pin, appCallbackFunction appCb);
	void setDigitalPin(int pin, bool state);
	bool isPinOutputInv(int pin);
	void attachDigPinCb(byte pin, appCallbackFunction pinCb);
	void detachDigPinCb(byte pin);
	int  initEspIO(void);
	bool setEspPinMode(byte pin, int mode);
	void digitalWriteCallback(byte port, int value);
	bool getIoLastReportedValue(byte pin);
protected:
	static int _mask;			// assume we only have PORT0
	static int _state;
	static int _old_inputs;
	static appCallbackFunction		_digitalPinCb[6];
};


class firmataApp : public espNode {
public:
	inline firmataApp(){ };
	firmataApp(char *ssid, char *pass);
	void setup(void);
	void process(void);
	void sysexCallback(byte command, byte argc, byte *argv);
	void hostConnectionCallback(byte state);
	void initTransport(void);
	void stringCallback(char *myString);
	bool isReady(void);
	void attachInitCb(initCallbackFunction initCb);
private:
	static initCallbackFunction _initNodesCb;
	static bool _init_done;
	static char *_ssid;   // your network SSID (name)
	static char *_pass;   // your network password
	uint32_t _start;
	uint8_t _init;
	uint8_t _retry;

};

firmataApp	firmApp(ssid, pass);

#endif
