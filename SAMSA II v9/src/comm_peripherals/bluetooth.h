// bluetooth.h created for project v8.1 on 05/25/2011 03:49:38

/*****************************************
* 
* Copyright peteco 2011
*
*
******************************************/

// compatibilidad con el módulo bluetooth BlueSMiRF de SparkFun (BlueRadios Modem)


#ifndef bluetooth_h
#define bluetooth_h

#include "serialcomm.h"

typedef unsigned char byte;


class Bluetooth {

	public:
		Bluetooth ();
		bool connect ();
		static void blue_process (byte instruccion, byte largo, byte* data);
		void send_load (byte pata);
		void send_msg (byte instruccion, byte largo, byte* data);   // "wrapper" de la Serialcomm::send_msg
	
	private:
		Serialcomm blue_serial;
		enum {NOT_PRESENT, UNCONNECTED, CONNECTED, MODE_FAST} status;
		byte ATcommand (const char *command, int timeout, byte numstrings, ...);
};


extern Bluetooth blue;

#endif