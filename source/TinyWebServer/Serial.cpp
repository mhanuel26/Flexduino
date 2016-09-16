#include "fsl_debug_console.h"
#include "Serial.h"

SerialConsole::SerialConsole(void){

}

size_t SerialConsole::write(uint8_t b)
{
	PUTCHAR(b);
}


int SerialConsole::read(void){
	return GETCHAR();
}
