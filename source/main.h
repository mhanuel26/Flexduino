#ifndef MAIN_H
#define MAIN_H

#define ARTIK_DISABLE				0
#define ARTIK_USE_REST_CLIENT 		1		// basic connectivity done
#define ARTIK_USE_WEBSOCK_CLIENT 	2		// idem
#define ARTIK_USE_MQTT_CLIENT 		3		// Not implemented yet

#define ARTIK_CONN_PROTOCOL		ARTIK_DISABLE	//ARTIK_USE_REST_CLIENT		//ARTIK_DISABLE	//	//	//ARTIK_USE_WEBSOCK_CLIENT

#if (ARTIK_CONN_PROTOCOL == ARTIK_USE_REST_CLIENT)
#define ARTIK_REST_CLIENT	1
#else
#define ARTIK_REST_CLIENT	0
#endif

#endif
