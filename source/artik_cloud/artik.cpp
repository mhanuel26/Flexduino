/**
 * FLEXDUIDO PLATFORM ARTIK CLOUD LIBRARY
 * Author: Manuel Alejandro Iglesias Abbatemarco.
 *
 *	licensed under LGPLv2.1
 *
 **/

#include "artik.h"
#include "WString.h"
#include "Serial.h"
#include <ArduinoJson.h>
#include <stdlib.h>
#include <string.h>

extern SerialConsole Serial;
#if (FIRMATA_MODE == FIRMATA_CLIENT)
extern IPAddress server_ip;
#endif

// Artik Cloud
String device_id = String("0cfb45de3d264b78ba3945a38f4ffd8b");
String device_token = String("d216241fe8414430a418898ec23699e1");
//String AuthorizationData = "Authorization: Bearer <YOUR DEVICE TOKEN>";
String AuthorizationData = "Authorization: Bearer " + device_token;


#if (ARTIK_CONN_PROTOCOL != ARTIK_DISABLE)

extern WebSocketsClient webSockClient;

#if (ARTIK_CONN_PROTOCOL == ARTIK_USE_WEBSOCK_CLIENT)
void webSocketArtikEvent(WStype_t type, uint8_t * payload, size_t length){
	Flexartik.webSocketArtikEvent(type, payload, length);
}
#endif


artikLand::artikLand(void){
#if (FIRMATA_MODE == FIRMATA_CLIENT)
	// Not using at this moment, since firmata as client stream mode only connects to one server at this time
	_nodePtr = &_node;
	_nodePtr->ip = server_ip;
	memset(_nodePtr->name, 0, sizeof(_nodePtr->name));
	strcpy(_nodePtr->name, "SmartLight");
#endif
}

// build a simple one nested json group response... sounds fancy...

int artikLand::build_group_msg(String groupName, String param, void* value, dtype type){
	using namespace ArduinoJson;
	StaticJsonBuffer<500> jsonBuffer; 				// reserve spot in memory

	JsonObject& root = jsonBuffer.createObject(); // create root objects
	root["sdid"] = device_id.c_str() ;
#if (ARTIK_CONN_PROTOCOL == ARTIK_USE_WEBSOCK_CLIENT)
	root["cid"] = "1234567890";
#endif
	root["type"] = "message";

	JsonObject& groupObj = root.createNestedObject("data");

	JsonObject& dataPair = groupObj.createNestedObject(groupName.c_str()); 	// create nested objects
	switch(type){
		case Boolean:
			dataPair[param.c_str()] = *(bool*)value;
			break;
		case Double:
			dataPair[param.c_str()] = *(double*)value;
			break;
		case Integer:
			dataPair[param.c_str()] = *(int*)value;
			break;
		case Long:
			dataPair[param.c_str()] = *(long*)value;
			break;
		case Strng:
			dataPair[param.c_str()] = (const char*)value;
			break;
	}

	memset(_buf, 0, sizeof(_buf));
	root.printTo(_buf, sizeof(_buf)); // JSON-print to buffer

	return (root.measureLength()); // also return length
}


int artikLand::build_simple_msg(String param, void* value, dtype type){
	using namespace ArduinoJson;
	StaticJsonBuffer<500> jsonBuffer; 				// reserve spot in memory

	JsonObject& root = jsonBuffer.createObject(); // create root objects
	root["sdid"] = device_id.c_str() ;
#if (ARTIK_CONN_PROTOCOL == ARTIK_USE_WEBSOCK_CLIENT)
	root["cid"] = "1234567890";
#endif
	root["type"] = "message";

	JsonObject& dataPair = root.createNestedObject("data"); 	// create nested objects

	switch(type){
		case Boolean:
			dataPair[param.c_str()] = *(bool*)value;
			break;
		case Double:
			dataPair[param.c_str()] = *(double*)value;
			break;
		case Integer:
			dataPair[param.c_str()] = *(int*)value;
			break;
		case Long:
			dataPair[param.c_str()] = *(long*)value;
			break;
		case Strng:
			dataPair[param.c_str()] = (const char*)value;
			break;
	}

	memset(_buf, 0, sizeof(_buf));
	root.printTo(_buf, sizeof(_buf)); // JSON-print to buffer

	return (root.measureLength()); // also return length
}


void artikLand::send_request(void){
	webSockClient.sendTXT(_buf, strlen(_buf));
}

void artikLand::send_request(int len){
	webSockClient.sendTXT(_buf, len);
}

void artikLand::send_status(bool stat){
	bool status = stat;
	build_simple_msg("status", &status);
	send_request();
}

void artikLand::toggleLED(uint8_t state){
	int len;
	bool status = (bool)state;
	len = build_simple_msg(STATE, &status);
	send_request(len);
}

void artikLand::handleSmartLightAction(bool state){
	int len;
	Serial.print("set SmartLight state to ");
	state ? Serial.println("ON") : Serial.println("OFF");
	setSmartLight(state);
	// now I should build a more complex response for the SmartLight state back to artik
	len = build_group_msg(SMART_LIGHT, STATE, &state);
	send_request(len);
}

void  artikLand::process_incoming_msg(uint8_t *msg){
	using namespace ArduinoJson;
	StaticJsonBuffer<1000> jsonBuffer; 							// reserve spot in memory

	JsonObject& root = jsonBuffer.parseObject((const char*)msg);

	const char* type = root["type"];
	Serial.print("type is ");
	Serial.println(type);
	if(!strncmp(type, "ping", strlen("ping"))){
		// ping received
		Serial.println("ping cmd received");
	}else if(!strncmp(type, "action", strlen("action"))){
		// action received
		if (root.containsKey("data"))
		{
			JsonObject& data = root["data"];
			JsonArray& actions = data["actions"];
			const char *action_name =  actions[0]["name"];
			Serial.println(action_name);
			uint8_t newState;
			if(!strncmp(action_name, SET_ON, strlen(SET_ON))){
				newState = 1;
				Serial.println("Turn Light On");
				toggleLED(newState);
			}else if(!strncmp(action_name, SET_OFF, strlen(SET_OFF))){
				newState = 0;
				Serial.println("Turn Light Off");
				toggleLED(newState);
			}else if(!strncmp(action_name, SET_SMART_LIGHT, strlen(SET_SMART_LIGHT))){
				Serial.println("set parameter on a Smart Light appliance");
				JsonObject& param = actions[0]["parameters"];
				bool OnState = param["OnState"].as<bool>();
				handleSmartLightAction(OnState);
			}else{
				return;
			}
		}
	}else{
		Serial.print("received unknown message ");
		Serial.println(type);
	}

}

#if (ARTIK_CONN_PROTOCOL == ARTIK_USE_WEBSOCK_CLIENT)

void artikLand::webSocketArtikEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
//            	{"type":"register", "sdid":"0cfb45de3d264b78ba3945a38f4ffd8b", "Authorization": "bearer d216241fe8414430a418898ec23699e1", "cid": "1234567890"}
                Serial.print("[WSSc] Connected to url: ");
                Serial.println((char *)payload);
                // Websocket connection is open
                String registerMessage = String();
                registerMessage += String("{\"type\":\"register\", \"sdid\": \"");
                registerMessage += device_id;
                registerMessage += String("\", \"Authorization\":\"bearer ");
                registerMessage += device_token;
                registerMessage += String("\", \"cid\": \"1234567890\"}");
                Serial.println("Register Message to send:");
                Serial.println(registerMessage);
//                '{"type":"register", "sdid":"'+device_id+'", "Authorization":"bearer '+device_token+'", "cid":"'+getTimeMillis()+'"}';
                webSockClient.sendTXT(registerMessage.c_str(), strlen(registerMessage.c_str()));
            }
            break;
        case WStype_TEXT:
            Serial.print("[WSc] get text: ");
            Serial.println((char *)payload);
            process_incoming_msg(payload);
            break;
        case WStype_BIN:
            Serial.print("[WSc] get binary length: ");
            Serial.println(length);
            break;
        default:
			break;
    }
}

#endif	// end of artik with websockets

#endif



