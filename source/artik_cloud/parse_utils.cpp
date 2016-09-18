 /** FLEXDUIDO PLATFORM ARTIK CLOUD LIBRARY UTILS 4 PARSING DATA
 * Author: Manuel Alejandro Iglesias Abbatemarco.
 *
 *	licensed under LGPLv2.1
 *
 **/

#include <ArduinoJson.h>
#include "parse_utils.h"

char buf[200];
extern String device_id;
extern String device_token;
extern String AuthorizationData;

//
//int loadBufferParam(String param, void *value, dtype type) {
//
//	using namespace ArduinoJson;
//	StaticJsonBuffer<200> jsonBuffer; 				// reserve spot in memory
//
//	JsonObject& root = jsonBuffer.createObject(); // create root objects
//	 root["sdid"] = device_id.c_str() ; 		// FIX
//	 root["type"] = "message";
//
//	JsonObject& dataPair = root.createNestedObject("data"); 	// create nested objects
//
//	switch(type){
//		case Boolean:
//			dataPair[param.c_str()] = *(bool*)value;
//			break;
//		case Double:
//			dataPair[param.c_str()] = *(double*)value;
//			break;
//		case Integer:
//			dataPair[param.c_str()] = *(int*)value;
//			break;
//		case Long:
//			dataPair[param.c_str()] = *(long*)value;
//			break;
//		case Strng:
//			dataPair[param.c_str()] = (const char*)value;
//			break;
//	}
//
//	root.printTo(buf, sizeof(buf)); // JSON-print to buffer
//
//	return (root.measureLength()); // also return length
// }


int loadBuffer(int insTemp, int insHumid ) {
	using namespace ArduinoJson;
	StaticJsonBuffer<200> jsonBuffer; // reserve spot in memory

	JsonObject& root = jsonBuffer.createObject(); // create root objects
	 root["sdid"] = device_id.c_str() ; // FIX
	 root["type"] = "message";

	JsonObject& dataPair = root.createNestedObject("data"); // create nested objects
	 dataPair["temp"] = insTemp;
	 dataPair["humid"] = insHumid;

	root.printTo(buf, sizeof(buf)); // JSON-print to buffer

	return (root.measureLength()); // also return length
 }




