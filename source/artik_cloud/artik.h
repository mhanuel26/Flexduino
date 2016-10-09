#ifndef ARTIK_H
#define ARTIK_H

#include "main.h"
#include "IPAddress.h"
#include "WebSocketsClient.h"
#include "parse_utils.h"
#include "firmata_app.h"

// some defines for Json Manifest

//{
//  "fields": [
//    {
//      "name": "picUrl",
//      "type": "CUSTOM",
//      "valueClass": "String",
//      "isCollection": false,
//      "description": "URL of jpg picture",
//      "tags": [
//        "JPG",
//        "PICTURE",
//        "FLEXIO"
//      ]
//    },
//    {
//      "name": "state",
//      "type": "CUSTOM",
//      "valueClass": "Boolean",
//      "isCollection": false,
//      "description": "state of light",
//      "tags": []
//    },
//    {
//      "name": "SmartLight",
//      "children": [
//        {
//          "name": "state",
//          "type": "CUSTOM",
//          "valueClass": "Boolean",
//          "isCollection": false,
//          "description": "Smart Light appliance state",
//          "tags": [
//            "LIGHT",
//            "iot node"
//          ]
//        },
//        {
//          "name": "ioNum",
//          "type": "CUSTOM",
//          "valueClass": "Integer",
//          "isCollection": false,
//          "description": "the I/O pin of smart light associated to state value",
//          "tags": []
//        }
//      ]
//    },
//    {
//      "name": "SmartDigitalSensor",
//      "children": [
//        {
//          "name": "ioNum",
//          "type": "CUSTOM",
//          "valueClass": "Integer",
//          "isCollection": false,
//          "description": "I/O number of the Digital Sensor",
//          "tags": []
//        },
//        {
//          "name": "state",
//          "type": "CUSTOM",
//          "valueClass": "Boolean",
//          "isCollection": false,
//          "description": "State of the Digital Sensor, ON or OFF as boolean",
//          "tags": []
//        }
//      ]
//    }
//  ],
//  "messageFormat": "json",
//  "actions": [
//    {
//      "name": "setOn",
//      "description": "Set state to On",
//      "parameters": [],
//      "isStandard": false,
//      "type": "CUSTOM"
//    },
//    {
//      "name": "setOff",
//      "description": "Set state to Off",
//      "parameters": [],
//      "isStandard": true,
//      "type": "CUSTOM"
//    },
//    {
//      "name": "setSmartLight",
//      "description": "set parameter on a smart light node",
//      "parameters": [
//        {
//          "name": "OnState",
//          "type": "CUSTOM",
//          "valueClass": "Boolean"
//        },
//        {
//          "name": "ioNum",
//          "type": "CUSTOM",
//          "valueClass": "Integer"
//        }
//      ],
//      "isStandard": false,
//      "type": "CUSTOM"
//    },
//    {
//      "name": "cfgSmartDigSensor",
//      "description": "configure Smart Digital Sensor to make push notifications to artik or not",
//      "parameters": [
//        {
//          "name": "active",
//          "type": "CUSTOM",
//          "valueClass": "Boolean"
//        },
//        {
//          "name": "ioNum",
//          "type": "CUSTOM",
//          "valueClass": "Integer"
//        }
//      ],
//      "isStandard": false,
//      "type": "CUSTOM"
//    }
//  ]
//}

// FIELD Defines
// Smart Light
#define SMART_LIGHT			"SmartLight"
// Smart Digital sensor
#define SMART_DIG_SENSOR	"SmartDigitalSensor"
// some commons
#define STATE 				"state"
#define IO_NUM				"ioNum"
// ACTIONS Defines
// Smart Light
#define SET_SMART_LIGHT			"setSmartLight"
#define LIGHT_STATE				"OnState"
// Smart Digital Sensor
#define CFG_SMART_DIG_SENSOR	"cfgSmartDigSensor"
#define ACTIVE					"active"
// some commons
#define SET_ON					"setOn"
#define SET_OFF					"setOff"


#define MAX_NODES			4		// Maximun number of nodes to connect

#define ARTIK_REFRESH_PERIOD	(10*60*1000)


typedef struct {
	IPAddress	ip;
	char name[20];
}WiFiNode;

typedef enum {
	Boolean,
	Double,
	Integer,
	Long,
	Strng
}dtype;

#if (ARTIK_CONN_PROTOCOL != ARTIK_DISABLE)

void webSocketArtikEvent(WStype_t type, uint8_t * payload, size_t length);

class artikLand : public firmataApp {
public:
	artikLand(void);
	void initCallbacks(void);
	void send_status(bool stat);		// just to test
	void webSocketArtikEvent(WStype_t type, uint8_t * payload, size_t length);
	void toggleLED(uint8_t state);
	int  build_simple_msg(String param, void* value, dtype type = Boolean);
	void espDigIOPinSensor(byte pin, bool value);
	char _buf[500];			// The JSON data
private:
#if (FIRMATA_MODE == FIRMATA_CLIENT)
	WiFiNode	_node;								// let's do a demo that connects to only one firmata server
#elif (FIRMATA_MODE == FIRMATA_SERVER)
	WiFiNode	_node[MAX_NODES];					// we need to adjust for server mode
#endif
	WiFiNode	*_nodePtr;
	void send_request(void);
	void send_request(int len);
	void handleSmartLightAction(int ioNum, bool state);
	void handleSmartDigitalSensorCfg(int ioNum, bool active);
	void sendDigSensorState(int pin, bool value);
	void process_incoming_msg(uint8_t *msg);
	int build_group_msg(String group, String param, void* value, dtype type = Boolean);
	int build_group_params_msg(String groupName, String param[], void* value[], dtype type[], int numParams);

};

artikLand Flexartik;

#endif	// End of Artik
#endif	// End of ARTIK_H
