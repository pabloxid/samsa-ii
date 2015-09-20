// bluetooth.cpp created for project v8.1 on 05/25/2011 03:55:24

/*****************************************
* Copyright Pablo Gindel 2011
******************************************/

#include "wiring.h"           // resulta que si no se incluye esto primero, la HardwareSerial salta porque faltan unos defs
#include "hardware.h"
#include "bluetooth.h"
#include <stdarg.h>
#include "mov_bajo_nivel.h"
#include "util.h"
#include "display.h"

Bluetooth blue;              // objeto bluetooth

Bluetooth::Bluetooth () {
	blue_serial = Serialcomm (&Serial, 115200, 3, 2, &blue_process);									// 3 bit instrucción, 2 bits largo
	status = NOT_PRESENT;
}

// este método probablemente desaparezca
// se conserva sólo para usar el simulador en Processing
void Bluetooth::send_load (byte pata) {                          // DEPRECATED
	byte data[3];
	for (byte anillo=0; anillo<3; anillo++) {
		data[anillo] = load[pata][anillo] + 100;    // para que sea siempre positivo
	}
	if (status == MODE_FAST) {blue_serial.send_msg (pata, 3, data);}   // esta comprobación evita que el buffer del modem se sature, 
																																				// en caso de que no esté conectado
}

void Bluetooth::send_msg (byte instruccion, byte largo, byte* data) {
	if (status == MODE_FAST) {blue_serial.send_msg (instruccion, largo, data);}
}

// maquinita de estados que gestiona la conexión al otro módulo bluetooth
// puede ser invocada en cualquier momento, por ejemplo si la conexión fue establecida desde afuera
bool Bluetooth::connect () {
	byte retries = 3;
	while (retries > 0) {
		switch (status) {
			case NOT_PRESENT:
				if (ATcommand("+++\r", 300, 1, "OK")) {status = CONNECTED;}       // esto contempla la posibilidad de que ya esté conectado, 
																																						 // pero nunca en modo "fast" 
				else if (ATcommand("AT\r", 300, 1, "OK")) {                    // está vivo
					if (ATcommand("ATSI,3\r", 500, 1, ",1")) {status = CONNECTED;}
					else {status = UNCONNECTED;}
				}
				else {retries --;}
				break;
			case UNCONNECTED: {
				retries --;
				byte conn = ATcommand("ATDM,00066601574D,1101\r", 3800, 2, "CONNECT", "NO ANSWER");  // bien hardcodeado el mac address
				if (conn == 1) {status = CONNECTED;}
				else if (conn == 0) {
					delay (300);
					status = NOT_PRESENT;
				}
				break;
			}
			case CONNECTED: {
				ATcommand("+++\r", 300, 0);
				byte mf = ATcommand("ATMF\r", 300, 2, "OK", "NO CARRIER");
				if (mf == 1) {status = MODE_FAST;}
				else if (mf == 2) {status = UNCONNECTED;}
				else {status = NOT_PRESENT;}
				break;
			}
			case MODE_FAST:
				return true;
		}
	}
	return false;
}

byte Bluetooth::ATcommand (const char *command, int timeout, byte numstrings, ...) {
	// capturar parámetros
	String key [numstrings];
	va_list args;
	va_start (args, numstrings);
	for (byte f=0; f<numstrings; f++) {
		key [f] = String (va_arg(args,const char*));
	}
	va_end(args);
	// enviar comando
	Serial.flush();
	Serial.print (command); 
	delay (timeout);            // wait response
	// leer respuesta
	byte len = Serial.available();
	if (len > 0) {
		char respuesta[len+1];
		for (byte f=0; f<len; f++){
			respuesta[f] = Serial.read();
		}
		respuesta[len] = NULL;
		String R = String (respuesta);
		// comparar con cada uno de los argumentos
		for (byte f=0; f<numstrings; f++) {
			if (R.indexOf(key[f])>-1) {return f+1;}
		}
	}
	return 0;
}
		
void Bluetooth::blue_process (byte instruccion, byte largo, byte* data) {
	
}