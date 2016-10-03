#ifndef ARTIK_H
#define ARTIK_H

#include "main.h"
#include "WebSocketsClient.h"
#include "parse_utils.h"


#define ARTIK_REFRESH_PERIOD	(10*60*1000)

typedef enum {
	Boolean,
	Double,
	Integer,
	Long,
	Strng
}dtype;

#if (ARTIK_CONN_PROTOCOL != ARTIK_DISABLE)

void webSocketArtikEvent(WStype_t type, uint8_t * payload, size_t length);

class artikLand{
public:
	artikLand(void);
	void send_status(bool stat);		// just to test
	void webSocketArtikEvent(WStype_t type, uint8_t * payload, size_t length);

	int build_simple_msg(String param, void* value, dtype type = Boolean);
	char _buf[200];			// The JSON data
private:

	void send_request(void);
	void process_incoming_msg(uint8_t *msg);

};

artikLand Flexartik;

#endif	// End of Artik
#endif	// End of ARTIK_H
