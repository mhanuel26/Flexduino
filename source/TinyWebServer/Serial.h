#ifndef SerialConsole_h
#define SerialConsole_h

#include <Stream.h>

class SerialConsole : public Print
{
public:
	SerialConsole();

	virtual size_t write(uint8_t byte);

	virtual int read(void);

	using Print::write;

};


#endif
