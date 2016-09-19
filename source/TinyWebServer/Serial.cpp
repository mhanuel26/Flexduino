#include "fsl_debug_console.h"
#include "Serial.h"

SerialConsole::SerialConsole(void){

}

size_t SerialConsole::write(uint8_t b)
{
	PUTCHAR(b);
	return 1;
}


int SerialConsole::read(void){
	return GETCHAR();
}


int SerialConsole::available(void){

	return 0;
}

int SerialConsole::peek(void){

	return 0;
}

void SerialConsole::flush(void){


}
