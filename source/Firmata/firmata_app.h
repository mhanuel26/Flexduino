#ifndef firmata_app_h
#define firmata_app_h

// Firmata Library
#if defined(WINC1500)
#define WIFI_101
#endif

#define SERVER_PORT		3030

#define OFF	false
#define ON	true

#include "Firmata.h"

extern char ssid[];
extern char pass[];
extern void printWifiStatus(void);

extern "C" {
// callback function types
typedef void (*appCallbackFunction)(bool);
}


class espNode {
public:
	void appSetDigitalPinCb(byte pin, appCallbackFunction appCb);
	void setDigitalPin(int pin, bool state);
	bool isPinOutputInv(int pin);
protected:
	int _state = 0;
	appCallbackFunction		_digitalPinCb[6];

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


private:
	static bool _init_done;
	static char *_ssid;   // your network SSID (name)
	static char *_pass;   // your network password
	uint32_t _start;
	uint8_t _init;
	uint8_t _retry;

};

firmataApp	firmApp(ssid, pass);

#endif
