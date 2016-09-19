#ifndef SerialConsole_h
#define SerialConsole_h

#include <Stream.h>

class SerialConsole : public Stream	//public Print
{
public:
	SerialConsole();

	inline void begin(void){  };
	inline void begin(long speed){  };
	virtual size_t write(uint8_t byte);

	virtual int read(void);
	virtual int available(void);
	virtual int peek(void);
	virtual void flush(void);
	using Print::write;

};


#endif
