/**
 * KINETICS NXP DESIGN CONTEST PROJECT - FLEXDUIDO PLATFORM
 * Author: Manuel Alejandro Iglesias Abbatemarco.
 *
 * PROJECT MAIN FEATURES:
 *
 * % ARDUINO WIFI 101 TCP/IP STACK
 *
 * % ARDUINO CORE LIBRARIES SUCH SERIAL, PRINT, ETC FOR READY TO INCLUDE OTHER ARDUINO CODE
 *
 * % ADVANCE WEB SERVER BASED ON MODIFIED ENHANCED TINYWEBSERVER LIBRARY
 *
 * % FAT FS ARDUINO LIBRARY (ELM CHAN + C++ ARDUINO LIBRARY)
 *
 * % WEBSOCKET LIBRARY BASED ON arduinoWebSockets LIBRARY
 *
 * % WEB CAMERA APPLICATION USING:
 *
 * - FLEXIO CAMERA DRIVER
 * - C++ API LIBRARY TO HANDLE TRANSFER FROM CAMERA DRIVER TO WEBSOCKETS CONNECTION USING JPEG ENCODING
 * - BINARY TRANSFERS USING WEBSOCKETS COUPLE WITH EXAMPLE APP USING HTML5 JAVASCRIPT WEB WORKERS IN FRONT END.
 *
 * % AUDIO LIBRARY FOR PLAYBACK / RECORDING
 *
 * - PLAYBACK USING DAC WITH DMA TRANSFERS FOR MINIMUN CPU INTERVENTION (ONLY SD CARD READ + PING PONG SWAP NEEDED)
 * - RECORDING USING DIGITAL MEMS SENSORS WITH DMA FOR MINIMUN CPU INTERVENTION.
 * - WORK ON PROGRESS TO TRANSMIT/RECEIVE AUDIO FROM WEB BROWSER.
 *
 *
 *	Flexduino Platform library is licensed under LGPLv2.1
 *
 **/
#include "fsl_port.h"

#include "main.h"
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#if defined(SIPSERVER)
#include <SipServer.h>
#endif
#include <WiFi101.h>
extern "C" {
#include "flexio_ov7670.h"
#include "bsp/include/nm_bsp_mk82fn256.h"
}
#include <assert.h>
#include <stdbool.h>
// WINC1500 MAC 	F8:F0:05:F0:B8:C0
// FatFs support includes
#include "sdCardSPI.h"
#include "FatFs/ff.h"
#include "FatFs.h"
#include "Serial.h"
#include "TinyWebServer.h"
#include "Flash.h"
#include "webApp.h"
#include "WString.h"

#include <stdarg.h>			// for the log wrapper
//#include "in.h"

// websockets support
#include "WebSockets.h"
#include "WebSocketsServer.h"

// ARTIK CLOUD SUPPORT
#include "artik.h"
#include "mm_iot.h"

// Test SSL client
#define SSL_CLIENT_TEST	0
#include "WiFiSSLClient.h"
#include "WiFiClient.h"

// web_cam
#define SNAP_TIMEOUT	5000	//	2 seconds timeout just huge to test with
#include "camera.h"
// flex_audio
#include "AudioFlex.h"
#include "fsl_edma.h"
// firmware updater (.ino as a library)
#include "firmware_updater.h"

// Example Firmata APP
#include "firmata_app.h"

SerialConsole Serial;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);

bool ws_send_file(uint8_t num, const char* filename);

static void put_rc (FRESULT rc);
void printWifiStatus();

char ssid[] = "YOUR_SSID_HERE";      // your network SSID (name)
char pass[] = "YOUR_AP_PASS";   // your network password

int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;

#define USE_FIRMATA			1
#define FIRMWARE_UPDATER	0
#define USE_WEBSOCK_SERVER	0			// Enable to use WebSocket server to stream pictures using javascript worker


#define IMGUR_CLIENT 	1
#define SERVER_ENABLE	0
#if defined(OV7670)
#define fps_avg			0			// change this one.
#else
#define fps_avg			0
#endif
#define netperf_test	0
#define HAVE_SD_CARD	1
#define SD_SPEED_TEST	1
#if SD_SPEED_TEST
#define SD_FILE_SIZE	512*80
static void testSdCard(FileFs *FileFsObj);		// A function to test SD card only accesable from within this file (proto here)
BYTE Buff[SD_FILE_SIZE]; /* File read buffer (80 SD card blocks to let multiblock operations (if file not fragmented) */
#endif

#if (SSL_CLIENT_TEST | ARTIK_REST_CLIENT | IMGUR_CLIENT)
//WiFiClient client;
WiFiSSLClient client;
//extern String AuthorizationData;
#endif

#if SSL_CLIENT_TEST
char server[] = "www.google.com";    // name address for Google (using DNS)
#elif ARTIK_REST_CLIENT
char server[] = "api.artik.cloud";
#endif

#if IMGUR_CLIENT
char imgur_url[] = "api.imgur.com";
#endif

int port = 443;
extern mm_iot	mmIoT;

#if netperf_test
//SipServer  sipServer;
WiFiServer perfServer(80);
#endif

#if fps_avg
float favg;
uint32_t nframes_time;
uint8_t	fcount = 0;		// frame count variable
uint32_t print_period;
#endif

int sipSock;
FileFs file;
boolean has_filesystem = true;

#if defined(SIPSERVER)
SipServer  sipServer;
#endif

extern TinyWebServer web;
extern WebSocketsServer webSocket;
extern AudioFlexClass AudioFlex;

#if (ARTIK_CONN_PROTOCOL == ARTIK_USE_WEBSOCK_CLIENT)
extern artikLand Flexartik;
WebSocketsClient webSockClient;
#endif

void take_picture_cb(void){
	Serial.println("callback start an image capture");
	camera.snapshot();
}

void init_firmata_nodes(void){
	Flexartik.initCallbacks();			// this do not do anything  - mostly for tests since we can attach callbacks from Artik cloud actions
	Flexartik.attachCameraSnap(take_picture_cb);		// we attach the camera callback but it also need the trigger action to setup from Artik Cloud
}

void init_bsp(void){
	/* Init board hardware. */
	BOARD_InitPins();
	BOARD_BootClockHSRUN();		// init at full speed 150 MHz
	BOARD_InitDebugConsole();
	BOARD_InitTimers();
}

void setup(void){
	init_bsp();
#if defined(OV7670)
	//added to camera firmware html- not sure if it was because USB needed or FlexIO
	CLOCK_SetClkOutClock(0x06);
	// our camera initialization
	camera.begin();
#endif
#if PLAYBACK_EN
	AudioFlex.begin();
#endif
	// init  message
	Serial.println();
	Serial.println("Initializing Flexduino Platform!");
	// Init SD card
#if HAVE_SD_CARD
	DWORD fre_sect, tot_sect;
	if(!file.begin()){
		has_filesystem = false;
	}
	if(has_filesystem){
		setAppHandlers();
		// some test to SD card
		tot_sect = file.capacity();
		fre_sect = file.free();
		PRINTF("%lu MB total drive space.\r\n"
			   "%lu MB available.\r\n",
			   fre_sect, tot_sect);
		PRINTF("SD card mount OK\r\n");
#if SD_SPEED_TEST
		testSdCard(&file);
#endif
	}
#endif

#if USE_FIRMATA
	firmApp.initTransport();
#else
	// check for the presence of the shield:
	if (WiFi.status() == WL_NO_SHIELD) {
		PRINTF("WiFi shield not present");
		// don't continue:
		while (true);
	}

	// attempt to connect to Wifi network:
	while ( status != WL_CONNECTED) {
		PRINTF("Attempting to connect to SSID: ");
		PRINTF(ssid);
		PRINTF("\r\n");
		// Connect to WPA/WPA2 network. Change this line if using open or WEP network:
		status = WiFi.begin(ssid, pass);
	}
	// you're connected now, so print out the status:
	printWifiStatus();

#endif

#if netperf_test
	// netperf test
	perfServer.begin();
	byte buf[1024];
#endif

#if SERVER_ENABLE
	//	init our web server
	web.begin();
#endif

#if (ARTIK_CONN_PROTOCOL == ARTIK_USE_WEBSOCK_CLIENT)			// Let's connect to Artik Cloud using Websockets
	// init webSockets client
	webSockClient.beginSSL("api.artik.cloud", 443, "/v1.1/websocket?ack=true", "");
	webSockClient.onEvent(webSocketArtikEvent);
#endif


#if USE_FIRMATA
	firmApp.attachInitCb(init_firmata_nodes);
	firmApp.setup();
#endif

#if USE_WEBSOCK_SERVER
	// init webSockets server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
#endif

#if defined(SIPSERVER)
	// init socket for osip
	sipServer = SipServer(5060);
	sipServer.begin();
#endif

#if SSL_CLIENT_TEST
	Serial.println("\nStarting connection to server...");
	// if you get a connection, report back via serial:
//	if (client.connect(server, 8883)) {
	if (client.connect("www.ssllabs.com", 443)) {
		Serial.println("connected to server");
		client.println("GET /ssltest/viewMyClient.html HTTP/1.1");
		client.println("Host: www.ssllabs.com");
		client.println("User-agent: ATWINC1500");
		client.println("Connection: close");
		client.println();


		// Make a HTTP request:
		// https://accounts.artik.cloud/authorize?client_id=9628eef2a00d43d89b757b8d34373588&response_type=code&redirect_uri=https://myapp.com/callback&state=abcdefgh
//		client.println("{\"status\":false}");
		//		client.println("POST /token HTTP/1.1");
//		client.println("Host: accounts.artik.cloud");
//		client.println("Authorization: Basic NzYwYzljNzc1ZWNkNGUzNmExNTE4ZTBkMzZkOWE5NmI6YjM2NTAzYjNjOTI1NDgzODhkZmI4M2JjMGM5YTViMDU=");
//		client.println("Content-Type: application/x-www-form-urlencoded");
//		client.println("");
//		client.println("grant_type=client_credentials");
//		client.println("Connection: close");
//		client.println();
//		client.println("\"Content-Type\": \"application/json\",");
//		client.println("\"Authorization\": \"Bearer d046fcd4209547c7831cfa5bb4c3ccd5\"");
//		client.println("GET /search?q=arduino HTTP/1.1");
//		client.println("Host: www.google.com");
//		client.println("Connection: close");
//		client.println();
	}
#endif
}

/*!
 * @brief Application entry point.
 */

int main(void) {
#if FIRMWARE_UPDATER
	init_bsp();			//init Serial port i.e.
	nm_bsp_init();
//	if (m2m_wifi_download_mode() != M2M_SUCCESS) {
//		Serial.println("Failed to put the WiFi module in download mode");
//		while (true);
//	}
#else
	// call setup
	setup();
#if PLAYBACK_EN
	uint32_t playtimeout;
	bool onetime = true;
	playtimeout = millis();
#endif

#if MEMS_TEST
	AudioFlex.turnRecordingOn();
	Serial.println("Recording Starting");
	AudioFlex.startMic();
	uint32_t start = millis();
#endif

#if (ARTIK_CONN_PROTOCOL != ARTIK_DISABLE)
	uint32_t start = millis();
	bool status = false;
#endif

#if (ARTIK_CONN_PROTOCOL ==	ARTIK_USE_REST_CLIENT)
	client.connect(server, port);
	delay(1000);
#endif

#if IMGUR_CLIENT
	// register a callback to send image to Imgur
	camera.registerSingleCaptureCb(sendImageCb);
#endif

#if IMGUR_CLIENT
#if defined(OV7670)
	// just to test Imgur
	camera.picturemode();
#endif
#endif

#endif	// Firmware updater

	for(;;){
#if FIRMWARE_UPDATER
//	updater_loop();
#else
#if defined(OV7670)
		if(camera.videomode() == bitbang){			// this is bitbang video mode, since we can only capture one image at a time
			if(camera.shotdone() == true){						// here we are getting frame and sending to websocket client
				camera.snapshot();
#if fps_avg
				if(fcount >= 9){									// we got here with count = 0 but has acquire one frame
					favg = 10000.0/(millis() - nframes_time);
					nframes_time = millis();
					fcount = 0;
					if(print_period - millis() >= 10000){
						print_period = millis();
						Serial.print("FlexDuino Camera FPS = ");
						Serial.println(favg, 1);
					}
				}else{
					fcount++;
				}
#endif
			}
		}
//		if(camera.videomode() == single_capture){
//			// handle single capture to RAM buffer
//			if(camera.shotdone() == true){
//				// this will be poll mode ...
//			}
//		}
		camera.process();			// keep an eye for  image capture completeness.
#endif

#if PLAYBACK_EN
	AudioFlex.process();
	// some tests with playback
	if(onetime){
		if((millis() - playtimeout) > 20000){
			AudioFlex.play((char*)"male.raw");
			onetime = false;
		}
	}
#endif
#if MEMS_TEST
		if(!AudioFlex.shouldStop()){
			if(millis() - start > 6000){
				AudioFlex.stopMic();
				Serial.println("Recording Stopped");
			}
		}
#endif
#if SERVER_ENABLE
		web.process();
		webSocket.process();
#endif
#if defined(SIPSERVER)
		sipServer.poll();
#endif
#if netperf_test
		WiFiClient client = perfServer.available();
		if(client){
			while (client.connected())
			{
			  if (client.available()) client.read(buf, 1024);
			}
		}
	    // close the connection:
	    client.stop();
#endif
#if SSL_CLIENT_TEST
	// if there are incoming bytes available
	// from the server, read them and print them:
	while (client.available()) {
		char c = client.read();
		Serial.write(c);
	}
	// if the server's disconnected, stop the client:
	if (!client.connected()) {
		Serial.println();
		Serial.println("disconnecting from server.");
		client.stop();
		// do nothing forevermore:
		while (true);
	}
#endif
#if (ARTIK_CONN_PROTOCOL == ARTIK_USE_WEBSOCK_CLIENT)
	// webSockets client loop
	webSockClient.loop();
	// test sending status every 10 minutes
//	if(millis() - start >= ARTIK_REFRESH_PERIOD){
//		start = millis();
//		Serial.println("Send status to Artik Cloud");
//		Flexartik.send_status(status);
//		status ^= true;
//	}
#endif
#if ARTIK_REST_CLIENT_4_TEST
    if (!client.connected()) {
      Serial.println(" error ");
    } else {
        Serial.println("Sending data");
        client.println("POST /v1.1/messages HTTP/1.1");
        client.println("Host: api.artik.cloud");
        client.println("Accept: */*");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println(AuthorizationData);
		// Automated POST data section
		client.print("Content-Length: ");
		// add the length
		client.println(build_simple_msg(String("status"), &status, Boolean));
		client.println();
		client.println(buf);

		delay(500);
		while (client.available()) {
			char c = client.read();
			Serial.write(c);
		}
//		Serial.println("POST /v1.1/messages HTTP/1.1");
//		Serial.println("Host: api.artik.cloud");
//		Serial.println("Accept: */*");
//		Serial.println("Content-Type: application/json");
//		Serial.println("Connection: close");
//		Serial.println(AuthorizationData);
//		// Automated POST data section
//		Serial.print("Content-Length: ");
//		// add the length
//		Serial.println(loadBufferParam(String("status"), &status, Boolean));
//		Serial.println();
//		Serial.println(buf);
    }
	delay(10*60*1000); // delay 10 min
    status ^= true;
#endif

#if USE_FIRMATA
	firmApp.process();
#endif

#endif	// Firmware Updater
	};				// end of endless loop
}


void printWifiStatus(void) {
	// print the SSID of the network you're attached to:
	Serial.print("SSID: ");
	Serial.println(WiFi.SSID());

	// print your WiFi shield's IP address:
	IPAddress ip = WiFi.localIP();
	Serial.print("IP Address: ");
	Serial.println(ip);

	// print the received signal strength:
	long rssi = WiFi.RSSI();
	Serial.print("signal strength (RSSI):");
	Serial.print(rssi);
	Serial.println(" dBm");
}


#if SD_SPEED_TEST
/* Stop with dying message */
// seriously this is only to be use for debugging
static void die(FRESULT rc)
{
	put_rc(rc);
	for(;;);
}
static void testSdCard(FileFs *FileFsObj){
    unsigned int i;
    UINT T;
    FileFs *FatFs = FileFsObj;

    for( i = 0; i < sizeof(Buff); i++) Buff[i] = i + i / 512; // fill the buffer with some data

    /****************************/
    int fh = FatFs->open("Data.bin", FA_WRITE | FA_CREATE_ALWAYS);
    if(fh < 0)
    	die((FRESULT)FatFs->error());
    T = millis();
    if(!FatFs->write(fh, Buff, sizeof(Buff)))
    	die((FRESULT)FatFs->error());
    T = millis() - T;
    float rate = (1000.0*SD_FILE_SIZE*8)/(T*1024);
    PRINTF("WRITE TOOK %d ms, rate = %.2f kbit/s\r\n", T, rate);
    if(!FatFs->close(fh))
    	die((FRESULT)FatFs->error());
    /****************************/
    memset(Buff, 0, sizeof(Buff));
    fh = FatFs->open("Data.bin", FA_READ);
    if(fh < 0)
    	die((FRESULT)FatFs->error());
    T = millis();
    if(!FatFs->read(fh, Buff, sizeof(Buff)))
    	die((FRESULT)FatFs->error());
    T = millis() - T;
    rate = (1000.0*SD_FILE_SIZE*8)/(T*1024);
    PRINTF("READ TOOK %d ms, rate = %.2f kbit/s\r\n", T, rate);
    if(!FatFs->close(fh))
    	die((FRESULT)FatFs->error());
    /****************************/
    if(!FatFs->openDir(""))
    	die((FRESULT)FatFs->error());
    for(;;){
    	if(!FatFs->nextFile()) break;
    	if(FatFs->isDirectory()){
    		PRINTF(" <dir> %s\r\n", FatFs->fileName());
    	}else{
    		PRINTF("%8d %s\r\n", FatFs->fileSize(), FatFs->fileName());
    	}
    }
//    /****************************/
}


static
void put_rc (FRESULT rc)
{
	const char *str =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
	int i;

	for (i = 0; i != rc && *str; i++) {
		while (*str++) ;
	}
	PRINTF("rc=%u FR_%s\r\n", (UINT)rc, str);
}

#endif


bool ws_send_file(uint8_t num, const char* filename) {

	int fh = file.open(filename, FA_READ);
	if (fh >=0) {
	  uint8_t buffer[1024];
	  size_t size;
	  while ((size = file.read(fh, buffer, sizeof(buffer))) > 0) {
		  webSocket.sendBIN(num, (const uint8_t*)buffer, size);
	  }
	  file.close(fh);
	  return true;
	}
	return false;
}


#if USE_WEBSOCK_SERVER
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            PRINTF("[%u] Disconnected!\r\n", num);
            break;
        case WStype_CONNECTED:
            {
            	webSocket.sendTXT(num, "start");
            }
            break;
        case WStype_TEXT:
        	PRINTF("[%u] get Text: %s\r\n", num, payload);
        	if(!strncmp((const char*)payload, "image_onfile", strlen("image_onfile"))){		// change this from image to image_onfile...
    			if(ws_send_file(num, "test.jpeg")){
    				PRINTF("Send JPEG image test\r\n");
    				webSocket.sendTXT(num, "end");
    			}else{
    				PRINTF("an error occur during image send test\r\n");
    				webSocket.sendTXT(num, "error");
    			}
        	}else if(!strncmp((const char*)payload, "image", strlen("image"))){
        		Serial.println("receive command for snapshot");
        		// need to inform about its ready, javascript will disable picture to SD file if not in video streaming mode
        	}else if(!strncmp((const char*)payload, "video_start", strlen("video_start"))){
        		Serial.println("receive command for start video");
        		camera.startvideo(num);				// this function is just faking video by starting a camera snapshot to capture YUV image and convert to jpeg
#if fps_avg
        		print_period = millis();
        		nframes_time = millis();
        		fcount = 0;
#endif
        	}else if(!strncmp((const char*)payload, "video_stop", strlen("video_stop"))){
        		Serial.println("receive command for stop video");
        		if(camera.videomode() == bitbang){
        			Serial.println("stopping video");
        			camera.stopvideo();
        		}else{
        			//another way to stop it? ...
        		}
        	}
            break;
        case WStype_BIN:
        	PRINTF("[%u] get binary length: %u\r\n", num, length);
        	uint8_t i;
        	for (i = 0; i < length; i++)
        	{
        	    if (i > 0) PRINTF(":");
        	    PRINTF("%02X", payload[i]);
        	}
        	PRINTF("\n");

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
        case WStype_ERROR:
        	//
        	break;
        default:
        	break;
    }

}
#endif


// EOF
