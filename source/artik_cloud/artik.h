#ifndef ARTIK_H
#define ARTIK_H

#include "WebSocketsClient.h"
#include "parse_utils.h"


#define ARTIK_USE_REST_CLIENT 		0		// basic connectivity done
#define ARTIK_USE_WEBSOCK_CLIENT 	1		// idem
#define ARTIK_USE_MQTT_CLIENT 		2		// Not implemented yet

#define ARTIK_CONN_PROTOCOL		ARTIK_USE_WEBSOCK_CLIENT

#define ARTIK_REFRESH_PERIOD	(10*60*1000)

void webSocketArtikEvent(WStype_t type, uint8_t * payload, size_t length);


//typedef struct{
//	char sdid[40];
//	char cid[20];
//	char type[15];				// "message"
//	char data_buf[200];			// The JSON data
//}Sahka;			// Message holder

class artikLand{
public:
	artikLand(void);
	void send_status(bool stat);		// just to test
	void webSocketArtikEvent(WStype_t type, uint8_t * payload, size_t length);

private:
	char _buf[200];			// The JSON data

	int build_simple_msg(String param, void* value, dtype type = Boolean);
	void send_request(void);
	void process_incoming_msg(uint8_t *msg);

};

artikLand Flexartik;

#endif
