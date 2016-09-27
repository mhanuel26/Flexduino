#ifndef ARTIK_H
#define ARTIK_H

#include "WebSocketsClient.h"
#include "parse_utils.h"

#define ARTIK_DISABLE				0
#define ARTIK_USE_REST_CLIENT 		1		// basic connectivity done
#define ARTIK_USE_WEBSOCK_CLIENT 	2		// idem
#define ARTIK_USE_MQTT_CLIENT 		3		// Not implemented yet


#define ARTIK_CONN_PROTOCOL		ARTIK_DISABLE	//ARTIK_USE_REST_CLIENT	//	//ARTIK_USE_WEBSOCK_CLIENT

#define ARTIK_REFRESH_PERIOD	(10*60*1000)

typedef enum {
	Boolean,
	Double,
	Integer,
	Long,
	Strng
}dtype;

#if (ARTIK_CONN_PROTOCOL == ARTIK_DISABLE)
	// We are not building ARTIK
#elif (ARTIK_CONN_PROTOCOL == ARTIK_USE_WEBSOCK_CLIENT)
void webSocketArtikEvent(WStype_t type, uint8_t * payload, size_t length);

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
#else
	int build_simple_msg(String param, void* value, dtype type);
#endif

#endif
