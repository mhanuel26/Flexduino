#ifndef firmata_app_h
#define firmata_app_h

// Firmata Library
#if defined(WINC1500)
#define WIFI_101
#endif

#include "Firmata.h"

extern char ssid[];
extern char pass[];
extern void printWifiStatus(void);

class firmataApp{
public:
	inline firmataApp(){ };
	inline firmataApp(char *ssid, char *pass){ _ssid = ssid; _pass = pass;};
	void setup(void);
	void process(void);
	void sysexCallback(byte command, byte argc, byte *argv);
	void hostConnectionCallback(byte state);
	void initTransport(void);
	void stringCallback(char *myString);
private:
	char *_ssid;   // your network SSID (name)
	char *_pass;   // your network password
	uint32_t _start;
};

firmataApp	firmApp(ssid, pass);

#endif
