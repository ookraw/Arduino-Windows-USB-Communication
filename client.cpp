/*
 ==================================================================================
 Name        : client.cpp
 Author      : Felix Baessler
 Version     : 03.11.2020
 Copyright   : Felix Baessler
 Website     : https://github.com/ookraw
 Description : see also server.ino

  Client

  This example (client.cpp) is a sketch of a Windows Client
  that communicates over
      USB serial
  with a Server (server.ino) running on an Arduino controller.

  For the purpose of this demo, we assume that Client and Server respect a
  simple, predefined protocol of interaction.

  Once the Arduino is physically connected to the MS Windows system via USB,
  the client can begin with the startup procedure:

  - find the active port among COM0 ... COM9
  - set the transmission parameters
    which will cause the Arduino to reset (wakeup)
  - activate the EOD character '#'
    that the Arduino must append to messages destined for this client
  - give the Arduino time to start after the reset

  Startup of Client and Server completed, the predefined message exchange
  can begin. The scenario chosen for this demo is documented in server.ino.

 ==================================================================================
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <Windows.h>
#define  NBUF 1000

bool serialRead (HANDLE h, char buf[], DWORD &received);
bool serialWrite(HANDLE h, char buf[], int &len);

// *********************************************************************************

int main() {
#define SM_LEN     8	// start message length     : "!start!#"
#define PARAM_LEN  7	// parameter message length : "111 222"

	HANDLE h;
	DCB dcb= { 0 }; 	// DCB initialization
	DWORD received;		// received number of bytes
	char buf[NBUF];		// received / sent data
	int  len;			// received / sent buffer length
	int  start;			// start position of the start message in buf[]
	char ip;			// port number 0 ... 9

	// EOD convention with Arduino : #
    // ===============================
	// const char eod= '#';

	// Arduino expects two integer parameters : 111 and 222
	char param[]= {'1', '1', '1', '\t', '2', '2', '2'};
	// current port to be scanned
	char port[]= {'C','O','M','0','\0','\0'};

	printf("client start\n");
	fflush(stdout);

	// scan ports from COM0 to COM9
    for (ip= 0; ip < 10; ip++) {

    	// set current port
    	// ----------------
    	port[3]= '0' + ip;
        h= NULL;
        h = CreateFile(port,				// port name
            GENERIC_READ | GENERIC_WRITE,	// read / write
            0, NULL,						// no sharing , no security
            OPEN_EXISTING, 					// Open existing port only
			FILE_ATTRIBUTE_NORMAL, 			// Non Overlapped I/O
			NULL);							// Null for Comm Devices

        // check current port
        // ------------------
        if(h == INVALID_HANDLE_VALUE) continue;	// --> next port
        // retrieve settings of current port
        if(!GetCommState(h, &dcb)) continue;	// --> next port

        // active port
        // ===========
    	printf("active port: %s \n", port);
		fflush(stdout);

    	// set transmission parameters
		// ---------------------------
        dcb.DCBlength= sizeof(dcb);
        dcb.BaudRate = CBR_9600;  			// Setting BaudRate = 9600
        dcb.ByteSize = 8;         			// Setting ByteSize = 8
        dcb.StopBits = ONESTOPBIT;			// Setting StopBits = 1
        dcb.Parity   = NOPARITY;  			// Setting Parity = None
        // reset Arduino                       <----- Arduino reset !!!
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        // EOD expected from Arduino           <----- Arduino EOD !!!
        dcb.EvtChar= '#';
        if (!SetCommState(h, &dcb)) {
    		printf("Error setting CommState, exit \n");
    		return EXIT_FAILURE;
        }
        // set timeouts
        // ------------

        // activate EOD char '#'
        // ---------------------
        if (!SetCommMask(h, EV_RXFLAG)) {
            printf("Error Setting CommMask, exit\n");
    		return EXIT_FAILURE;
        }

        // give Arduino time to start after reset
        // --------------------------------------
        Sleep(200);

        // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    	// get start message from Arduino
		// =================
        // first action on Arduino: send start message to the client
        // Arduino signals to be ready by a Serial.print("!start!#")
		// note that the message ends with the EOD character
        // note also that the returned buffer may contain foreign
        //      leading characters including '\0' or worse '#' !

		printf("get start message\n");
		fflush(stdout);
		// wait (FOREVER) until the message arrives (no timeout)
		if (serialRead (h, buf, received)) {
			len= (int)received;
			printf("start buffer contains %d bytes\n", len);
			// set begin of the start message in buf[]
			start= len - SM_LEN;
			if (start >= 0) {
				printf("start message:\n");
				printf("> \n");
				buf[len]= '\0';
				printf("%s< \n", buf + start);
				fflush(stdout);
			} else {
	            printf("Error start message, exit\n");
	    		return EXIT_FAILURE;
			}
		} else {
            printf("Error serialRead, exit\n");
    		return EXIT_FAILURE;
		}

		// send parameters to Arduino
		// ===============
        // second action on Arduino: receive 2 integers from the client
		printf("send parameters\n");
		fflush(stdout);
		len= PARAM_LEN;
		// send the two integers 111 and 222 to Arduino
		// (without EOD for write operations)
		if (serialWrite (h, param, len)) {
			printf("parameters sent: %d bytes\n", len);
			fflush(stdout);
		} else {
            printf("Error serialWrite, exit\n");
    		return EXIT_FAILURE;
		}
		fflush(stdout);

        // get result from Arduino
		// ==========
        // third action on Arduino: send result message back to the client
		// Arduino must mark the EOD by a Serial.print("#")
		printf("get result\n");
		fflush(stdout);
		// wait (FOREVER) until the message arrives
		if (serialRead (h, buf, received)) {
			len= (int)received;
			printf("result contains %d bytes:\n", len);
			printf("> \n");
			buf[len]= '\0';
			printf("%s< \n", buf);
			goto END;
		} else {
            printf("Error serialRead, exit\n");
    		return EXIT_FAILURE;
		}

    	CloseHandle(h);
    }   // scan ports loop

END:
	printf("client end\n");
	CloseHandle(h);
    return 0;
}

// $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

bool serialRead (HANDLE h, char buf[], DWORD &received) {

	// read buffer from Arduino
	// ========================

	DWORD   errors;
	COMSTAT status;
	DWORD   to_receive;
    DWORD   dwEventMask;

    // wait FOREVER for the EOD character '#'
	// --------------------------------------
    // (note: no timeout available for WaitCommEvent!)
    printf("wait (FOREVER!)\n");
	fflush(stdout);
    WaitCommEvent(h, &dwEventMask, NULL); //Wait for the character to be received

	// get the number of bytes ready to be read
	// ----------------------------------------
	printf("status\n");
	fflush(stdout);
	ClearCommError(h, &errors, &status);
	to_receive= status.cbInQue;

	// read the available bytes
	// ------------------------
	printf("read %d bytes\n", (int)to_receive);
	fflush(stdout);
	received= 0;
	if (to_receive > 0) {
		ReadFile(h, buf, to_receive, &received, NULL);
		return(true);
	}

	return(false);
}

bool serialWrite(HANDLE h, char buf[], int &len) {

	// write buffer to Arduino
	// =======================

	DWORD sent;
	DWORD to_send;
    DWORD dwEventMask;     // Event mask to trigger

	// write
	// -----
	to_send= (DWORD)len;
	sent=    0;
	WriteFile(h, buf, to_send, &sent, NULL);
	// WAIT, otherwise problem with the following read!
    WaitCommEvent(h, &dwEventMask, NULL);

	if (to_send == sent) return(true);
	else return(false);
}

/*
*/

