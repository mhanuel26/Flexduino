
#include "main.h"
#include "mm_iot.h"
#include "Stream.h"
#include "Serial.h"
#include "Base64.h"
#include "camera.h"
#include "WiFiSSLClient.h"
#include "WiFiClient.h"
#include <ArduinoJson.h>
#include "artik.h"


#define MAX_UPLOAD_RESPONSE_SIZE	500
#define JSON_BUFFER_SIZE			3000
#define IMGUR_URL		"api.imgur.com"
#define ARTIK_URL		"api.artik.cloud"

extern uint8_t imgBuffer[10000];
String imgur_token = String("YOUR_IMGUR_TOKEN");
String client_ID = String("YOUR_CLIENT_ID");
String AuthorizationDataImgur =  "Authorization: Bearer " + imgur_token;
//String AuthorizationDataImgur =  "Authorization: Client-ID " + client_ID;
uint8_t __attribute__((aligned(32), section(".myRAM"))) tmpBuf[10500];

extern String AuthorizationData;			// auth from artik
extern webCam camera;
extern SerialConsole Serial;
extern WiFiSSLClient client;

void sendImageCb(void){
	mmIoT.sendImgurImageCb();
}

void mm_iot::uploadImageUrlDoneCb(void){
	char url[40];

	memset(url, 0, sizeof(url));
	strcat(url, "http://i.imgur.com/");
	strcat(url, _id);
	strcat(url, ".jpg");

#if (ARTIK_CONN_PROTOCOL == ARTIK_USE_REST_CLIENT)
	client.connect(ARTIK_URL, 443);
	delay(1000);
    if (!client.connected()) {
      Serial.println(" error connecting");
    } else {
        Serial.println("Sending data to Artik Cloud!");
        client.println("POST /v1.1/messages HTTP/1.1");
        client.println("Host: api.artik.cloud");
        client.println("Accept: */*");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println(AuthorizationData);
		// Automated POST data section
		client.print("Content-Length: ");
		// add the length
		client.println(Flexartik.build_simple_msg(String("picUrl"), url, Strng));
		client.println();
		client.println(Flexartik._buf);
		long int start = millis();
		for(;;){
			if(millis() - start >= 5000)
				break;
			while (client.available()) {
				char c = client.read();
				Serial.write(c);
			}
		}
    }
#elif (ARTIK_CONN_PROTOCOL == ARTIK_USE_WEBSOCK_CLIENT)
    sendImgUrl(url);
#endif

}

int mm_iot::loadImageInJson(void){
	using namespace ArduinoJson;

	StaticJsonBuffer<6000> jsonBuffer; 							// reserve spot in memory
	JsonObject& root = jsonBuffer.createObject(); 				// create root objects
	int encodedLen = base64_enc_len(camera.getImgSize());
	char *encoded = (char*)malloc(encodedLen*sizeof(char));
	char *input = (char*)&imgBuffer[0];
	uint16_t imgSize = camera.getImgSize();
	base64_encode(encoded, input, imgSize);
	root["image"] = encoded;			//"Testing the Fucking Length Issue";	//
	root["album"] = "z4XS8";			// Id of the album - can be seen on URL page of album
	root["type"] = "base64";
//	root["name"] = "testFlexduino.jpg";
	root["title"] = "Flex";
	memset((void *)tmpBuf, 0, sizeof(tmpBuf));
	root.printTo((char *)tmpBuf, sizeof(tmpBuf)); // JSON-print to buffer
	int length =  root.measureLength();
	free(encoded);
	return (length); 							// also return length
}


// This function works but actually it seems there is a little bug with client reception of winc1500 library
// so I am just going to parse a  unique value I am interested which is the id meanwhile the bug gets fixed

//void parseImgurUpload(char *json){
//	using namespace ArduinoJson;
//	int size = strlen(json);
//
//	char jsonBuf[size];
//	Serial.print("JSON raw size is = ");
//	Serial.println(size);
//	strncpy(jsonBuf, json, size);
//	StaticJsonBuffer<1500> jsonBuffer;
////	DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);
//	JsonObject& root = jsonBuffer.parseObject(jsonBuf);
//	Serial.println();
//	// Test if parsing succeeds.
//	if (!root.success()) {
//		Serial.println("parseObject() failed");
//		return;
//	}
//	const char*id = root["data"]["id"];
//	const char*link = root["data"]["link"];
//	Serial.println();
//
//	Serial.print("Picture _id = ");
//	Serial.println(id);
//	Serial.print("Picture Link = ")
//	Serial.println(link);
//}

void mm_iot::parseImgurUpload(char *json){
	char *ptr1, *ptr2;
	int len;
//	char *id;

	if(!(ptr1 = strstr(json, "id\":")))
		return;
	ptr1 += strlen("id\":") + 1;
	ptr2 = strstr(ptr1, "\",\"");
	len = ptr2 - ptr1;
//	id = (char*)malloc(len+1);
//	if(!id)
//		return;
//	memset(id, 0, len+1);
	memset(_id, 0, sizeof(_id));
	strncpy(_id, ptr1, len);
	Serial.print("Picture ID = ");
	Serial.println(_id);
}

void mm_iot::sendImgurImageCb( void ){
	boolean currentLineIsBlank = true;
	boolean chase4JsonStart = false;
	boolean chase4JsonEnd = false;
	boolean postSuccess = false;
//	char json[600];
//	char *jsonPtr = &json[0];
	char rxBuf[5000];
	char *rxBufPtr = &rxBuf[0];
	memset(rxBuf, 0, sizeof(rxBuf));
	char *jsonPtr = (char*)malloc(MAX_UPLOAD_RESPONSE_SIZE);
	char *jsonAdderPtr = jsonPtr;
	uint16_t idx = 0;
	uint8_t nested;
	Serial.println("trying to connect to server");
//	memset(json, 0, sizeof(json));
	memset(jsonPtr, 0, MAX_UPLOAD_RESPONSE_SIZE);
	if(client.connect(IMGUR_URL, 443)){
		Serial.println(" connected to Imgur - Send image");
		// client POST to Imgur API version 3
		client.println("POST /3/image.json HTTP/1.1");
		client.println("Host: api.imgur.com");
		client.println("Accept: */*");
		client.println("Content-Type: application/json");
		client.println("Connection: close");
		client.println(AuthorizationDataImgur);
		// Automated POST data section
		client.print("Content-Length: ");
		// add the length
		int n = loadImageInJson();
		client.println(n);
		client.println();
		char *ptr = (char*)&tmpBuf[0];
		// transmit without overflowing the tx buffer
		int k;
		while(n){
			if(n < 500)
				k = client.write(ptr, n);
			else
				k = client.write(ptr, 500);
			if(k == 0){
				delay(1);
				continue;
			}else{
				n  = n - k;
				ptr += k;
			}
		}
		client.println();
	}
	Serial.println("Send Image done");
	long start = millis();
	for(;;){							// this timeout is needed
		if(millis() - start >= 5000)
			break;
		while (1) {
			int rdy = client.available();
			if (!rdy)
				break;
			start = millis();
			client.read((uint8_t*)rxBufPtr, rdy);
			rxBufPtr += rdy;
		}
	}

	char *idxPtr = &rxBuf[0];
	uint16_t len = rxBufPtr - idxPtr;
	for(int j=0; j<len; j++){
//		char c = client.read();
		char c = *idxPtr++;
		Serial.write(c);
		if(idx >= 100){
			Serial.println();
			idx = 0;
		}
		if(c == '{' && chase4JsonStart){
//				json[idx++] = c;
			idx++;
			*jsonAdderPtr++ = c;
			chase4JsonStart = false;
			chase4JsonEnd = true;
			nested = 1;
			continue;
		}
		if(chase4JsonEnd){
			if(c == '}'){
				nested--;
				if(nested == 0){
					chase4JsonEnd = false;
					postSuccess = true;
				}
			}else if(c == '{'){
				nested++;
			}
//				json[idx++] = c;	// add the character to json string in any case...
			idx++;
			*jsonAdderPtr++ = c;
		}
		if (c == '\n' && currentLineIsBlank){
			chase4JsonStart = true;
			currentLineIsBlank = false;
			continue;
		}
		if (c == '\n') {
		  currentLineIsBlank = true;
		}else if (c != '\r') {
		  currentLineIsBlank = false;
		}

	}
	parseImgurUpload(jsonPtr);
    if (client.connected()) {
      Serial.println("disconnecting from server");
      client.stop();
    }
    free(jsonPtr);
    uploadImageUrlDoneCb();
}
