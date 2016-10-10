
// Firmata Library
#include "firmata_app.h"
#include <WiFi101.h>
#include "Serial.h"
#include "WiFiStream.h"
#include "WiFiClientStream.h"
#include "WiFiServerStream.h"
#include "firmataDebug.h"

#define PORT0	0

extern SerialConsole Serial;


bool firmataApp::_init_done = false;
char * firmataApp::_ssid;   // your network SSID (name)
char * firmataApp::_pass;   // your network password
initCallbackFunction firmataApp::_initNodesCb = NULL;


int espNode::_mask = 0;		// this will be a problem? when migaret to several clients?
int espNode::_state = 0;
int espNode::_old_inputs = 0;
appCallbackFunction		espNode::_digitalPinCb[6] = { NULL, NULL, NULL, NULL, NULL, NULL};
camTriggerCbFnct		espNode::_pinCamPosTriggerCb[6] = { NULL, NULL, NULL, NULL, NULL, NULL};
camTriggerCbFnct		espNode::_pinCamNegTriggerCb[6] = { NULL, NULL, NULL, NULL, NULL, NULL};

#define DEBUG_FIRMATA	1

// Test with our Linux Host
//IPAddress server_ip(192, 168, 1, 5);
//uint16_t port #define D7		0b10000000= 5000;
// Test with ESP12
IPAddress server_ip(192, 168, 1, 70);
//IPAddress server_ip2(192, 168, 1, 75);
//static uint16_t port = 3030;

WiFiClientStream stream(server_ip, SERVER_PORT);
//WiFiClientStream stream2(server_ip2, SERVER_PORT);

//FirmataClass Firmata;
//FirmataClass Firmata2;


static void hostConnectionCb(byte state){
	firmApp.hostConnectionCallback(state);
}


static void sysexCb(byte command, byte argc, byte *argv){
	firmApp.sysexCallback(command, argc, argv);
}

firmataApp::firmataApp(char *ssid, char *pass){
	_ssid = ssid;
	_pass = pass;
	_initNodesCb = NULL;
	espNodeInit();
}

void firmataApp::stringCallback(char *myString)
{
	Firmata.sendString(myString);
}

void firmataApp::sysexCallback(byte command, byte argc, byte *argv)
{
	switch (command) {
		case ANALOG_MAPPING_QUERY:
			Firmata.write(START_SYSEX);
			Firmata.write(ANALOG_MAPPING_RESPONSE);
			for (byte pin = 0; pin < TOTAL_PINS; pin++) {
				Firmata.write(IS_PIN_ANALOG(pin) ? PIN_TO_ANALOG(pin) : 127);
			}
			Firmata.write(END_SYSEX);
			break;
		default:
//			Firmata.sendSysex(command, argc, argv);
			break;
	}
}


/*
 * Called when a TCP connection is either connected or disconnected.
 * TODO:
 * - report connected or reconnected state to host (to be added to protocol)
 * - report current state to host (to be added to protocol)
 */
void firmataApp::hostConnectionCallback(byte state)
{
  switch (state) {
    case HOST_CONNECTION_CONNECTED:
      DEBUG_PRINTLN( "TCP connection established" );
      break;
    case HOST_CONNECTION_DISCONNECTED:
      DEBUG_PRINTLN( "TCP connection disconnected" );
      break;
  }
}

void firmataApp::initTransport(void)
{
	if(!_init_done){
		// This statement will clarify how a connection is being made
		DEBUG_PRINT( "StandardFirmataWiFi will attempt a WiFi connection " );
#if defined(WIFI_101)
		DEBUG_PRINTLN( "using the WiFi 101 library." );
#elif defined(ARDUINO_WIFI_SHIELD)
		DEBUG_PRINTLN( "using the legacy WiFi library." );
#elif defined(ESP8266_WIFI)
		DEBUG_PRINTLN( "using the ESP8266 WiFi library." );
#elif defined(HUZZAH_WIFI)
		DEBUG_PRINTLN( "using the HUZZAH WiFi library." );
		//else should never happen here as error-checking in wifiConfig.h will catch this
#endif  //defined(WIFI_101)

		// Configure WiFi IP Address
#ifdef STATIC_IP_ADDRESS
		DEBUG_PRINT( "Using static IP: " );
		DEBUG_PRINTLN( local_ip );
#if defined(ESP8266_WIFI) || (defined(SUBNET_MASK) && defined(GATEWAY_IP_ADDRESS))
		stream.config( local_ip , gateway, subnet );
#else
		// you can also provide a static IP in the begin() functions, but this simplifies
		// ifdef logic in this sketch due to support for all different encryption types.
		stream.config( local_ip );
#endif
#else
		DEBUG_PRINTLN( "IP will be requested from DHCP ..." );
#endif

		stream.attach(hostConnectionCb);

		// Configure WiFi security and initiate WiFi connection
#if defined(WIFI_WEP_SECURITY)
		DEBUG_PRINT( "Attempting to connect to WEP SSID: " );
		DEBUG_PRINTLN(ssid);
		stream.begin(ssid, wep_index, wep_key);
#elif defined(WIFI_WPA_SECURITY)
		DEBUG_PRINT( "Attempting to connect to WPA SSID: " );
		DEBUG_PRINTLN(ssid);
		stream.begin(ssid, wpa_passphrase);
#else                          //OPEN network
		DEBUG_PRINTLN( "Attempting to connect to open SSID: " );
		DEBUG_PRINTLN(_ssid);
		stream.begin(_ssid, _pass);
#endif //defined(WIFI_WEP_SECURITY)
		DEBUG_PRINTLN( "WiFi setup done - Wait for connection to AP" );

		// Wait for connection to access point to be established.
		while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		DEBUG_PRINT(".");
		}
		printWifiStatus();
		_init_done = true;			// we can only init wifi network once
	}
}

bool firmataApp::isReady(void){
	return (bool)_init;
}


void firmataApp::setup(void){
//	Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
//	Firmata.attach(STRING_DATA, stringCallback);
//	Firmata.attach(START_SYSEX, sysexCallback);
//	Firmata.begin(115200);


	Firmata.setFirmwareVersion(FIRMATA_FIRMWARE_MAJOR_VERSION, FIRMATA_FIRMWARE_MINOR_VERSION);
//	Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
	Firmata.attach(DIGITAL_MESSAGE, [](byte port, int value) { return firmApp.digitalWriteCallback(port, value); } );
//	Firmata.attach(REPORT_ANALOG, reportAnalogCallback);
//	Firmata.attach(REPORT_DIGITAL, reportDigitalCallback);
//	Firmata.attach(SET_PIN_MODE, setPinModeCallback);
//	Firmata.attach(SET_DIGITAL_PIN_VALUE, setPinValueCallback);
//	Firmata.attach(START_SYSEX, sysexCb);
//	Firmata.attach(SYSTEM_RESET, systemResetCallback);

//	ignorePins();

	// Initialize Firmata to use the WiFi stream object as the transport.
	Firmata.begin(stream);
	delay(1000);				// For some reason, perhaps winc1500, if I continue without this delay, packets wont send out for a second

	_start = millis();
	_init = 0;
	_retry = 0;
	_state = 0;
//	Firmata.sendSetPinMode(2, OUTPUT);
//	Firmata.sendSetPinMode(4, INPUT);
//	Firmata.reportDigitalPin(4, 1);
//	setSmartLight(OFF);
//	Serial.println("setup firmata");
//	Firmata.sendQueryFirmware();
//	systemResetCallback();  // reset to default config
}


void firmataApp::attachInitCb(initCallbackFunction initCb){
	_initNodesCb = initCb;
}

void firmataApp::process(void){
	while (Firmata.available()) {
		Firmata.processInput();
	}
	stream.maintain();

	if(!_init){
		if(initEspIO()){
			if(_initNodesCb != NULL)
				_initNodesCb();
			_init = 1;
			Serial.print("number of retries = ");
			Serial.println(_retry);
		}else{
			_retry++;
		}
	}

//////	 just for tests
//	if((millis() - _start) > 1000){
//		Serial.println("toggle LED test");
//		Firmata.sendDigitalPort(PORT0, _state);
//		_start = millis();
//		_state ^= D2;
//	}

}


// ********************************************
// *********** 	espNode class   ***************
// ********************************************

void espNode::espNodeInit(void){
	//testing with static methods
//	for(uint8_t j=0; j < sizeof(_digitalPinCb); j++){
//		_digitalPinCb[j] = NULL;
//	}
//	_state = 0;
//	_mask = 0;
//	_old_inputs = 0;
}

bool espNode::isPinOutputInv(int pin){
	byte lport = (byte)(pin & 0x7f);
	if(lport && D2)
		return true;
	return false;
}

void espNode::setDigitalPin(int pin, bool value){
	if(isPinOutputInv(pin))
		value ? _state &= ~pin : _state |= pin;
	else
		value ? _state |= pin : _state &= ~pin;
	Serial.print("Set Digital Pin ");
	Serial.println(_state, BIN);
	Firmata.sendDigitalPort(PORT0, _state);
}


void espNode::attachDigPinCb(byte pin, appCallbackFunction pinCb){
	setEspPinMode(pin, INPUT);		// I only consider a callback on an INPUT pin...
	Firmata.reportDigitalPin(pin, 1);
	_digitalPinCb[pin] = pinCb;
}



void espNode::detachDigPinCb(byte pin){
	Firmata.reportDigitalPin(pin, 0);
	_digitalPinCb[pin] = NULL;
}

void espNode::attachDigPinCamTriggerCb(byte pin, camTriggerCbFnct camCb, trg_type type){
	switch(type){
		case trigger_disable:
		default:
			_pinCamPosTriggerCb[pin] = NULL;
			_pinCamNegTriggerCb[pin] = NULL;
		break;
		case trigger_rising:
			_pinCamPosTriggerCb[pin] = camCb;
			_pinCamNegTriggerCb[pin] = NULL;
		break;
		case trigger_falling:
			_pinCamPosTriggerCb[pin] = NULL;
			_pinCamNegTriggerCb[pin] = camCb;
		break;
	}
}

void espNode::dettachDigPinCamTriggerCb(byte pin){
	_pinCamPosTriggerCb[pin] = NULL;
	_pinCamNegTriggerCb[pin] = NULL;
}

bool espNode::setEspPinMode(byte pin, int mode){
	bool res = Firmata.sendSetPinMode(pin, mode);
	if(res && (mode == INPUT)){
		_mask |= (0x01 << pin);
	}
	return res;
}

int espNode::initEspIO(void){
	bool res = setEspPinMode(nD2, OUTPUT);
	if(res){
		// set the blue led to OFF state
		setDigitalPin(D2, OFF);
		// this should be application dependent...
//		setEspPinMode(nD4, INPUT);
//		Firmata.reportDigitalPin(nD4, 1);
		return 1;
	}
	return 0;
}

bool espNode::getIoLastReportedValue(byte pin){
	byte mask = 0x1;
	byte value;

	mask = mask << pin;
	value = mask & _old_inputs;
	if(value)
		return true;
	return false;
}

void espNode::digitalWriteCallback(byte port, int value){
#if DEBUG_FIRMATA
	Serial.print("port = ");
	Serial.println(port, BIN);
	Serial.print("value = ");
	Serial.println(value, BIN);
#endif
	// now the handler using callbacks
	int newValue = value & _mask;
	int mask = _old_inputs^newValue;
	if(mask){
		_old_inputs = newValue;
		for(byte j=0; j<6; j++){
			if(mask & 0x1){
				if(_digitalPinCb[j] == NULL)
					break;				// do not call NULL please
				if(newValue&0x1){
					_digitalPinCb[j](j, true);
					if(_pinCamPosTriggerCb[j] != NULL)
						_pinCamPosTriggerCb[j]();
				}else{
					_digitalPinCb[j](j, false);
					if(_pinCamNegTriggerCb[j] != NULL)
						_pinCamNegTriggerCb[j]();
				}
			}
			mask = mask >> 1;
			newValue = newValue >> 1;
		}
	}
}
