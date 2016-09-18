#ifndef PARSE_UTIL_H
#define PARSE_UTIL_H

#include "WString.h"

typedef enum {
	Boolean,
	Double,
	Integer,
	Long,
	Strng
}dtype;

//int loadBufferParam(String param, void *value, dtype type);
int loadBuffer(int insTemp, int insHumid );


#endif
