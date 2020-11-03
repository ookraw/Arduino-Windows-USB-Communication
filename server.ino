/*
 ==================================================================================
 Name        : server.ino
 Author      : Felix Baessler
 Version     : 03.11.2020
 Copyright   : Felix Baessler
 Website     : https://github.com/ookraw
 Description : see also client.cpp

  Server

  This example (server.ino) is a sketch of an Arduino server 
  that communicates over 
      USB serial 
  with a Client (client.cpp) running on a MS Windows system.

  For the purpose of this demo, we assume that Server and Client respect a simple, 
  predefined protocol of interaction.

  The following scenario has been chosen:
  
  - the client contacts the server, thereby causing a reset on the Arduino; 
    thereafter the client waits for the start message from the server
  - after the wakeup (reset), the server starts with a first action:
    1st action: send start message to the client
  - terminating the wait (WaitCommEvent), the client receives the start message, 
    sends a parameter message consisting of two integers and  
    waits for the reply from the server
  - the server continues with the second and third action:
    2nd action: receive 2 integers from the client
    3rd action: send result message back to the client
    thereafter the server continues blinking forever
  - terminating the wait, the client receives the result message and 
    terminates execution. 
  
 ==================================================================================
*/

#define SM_LEN          8    // start message length : "!start!#"
#define LED            13
#define SERIAL_BAUD  9600
int i1;
int i2;

// Server Wakeup
// =============
void setup() {
  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(SERIAL_BAUD);
  delay(100);
  
  // first action: send start message to the client
  // ----------------------------------------------
  // predefined start message, including the EOD character
  Serial.print("!start!#"); 
  Serial.flush();
  
  // second action: receive 2 integers from the client
  // -------------------------------------------------
  while (Serial.available() <= 0) {
    delay(500);
  }
  i1= Serial.parseInt();
  i2= Serial.parseInt();
  // check parameters
  if ((i1 == 111) && (i2 == 222)) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
    delay(5000);                       // wait for 5 seconds
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off 
  }
  
  // third action: send result message back to the client
  // ----------------------------------------------------
  Serial.println("line1: aaaaaaaaaaaaaaaaaaaaaaaaa");  
  Serial.println("line2: bbbbbbbbbbbbbbbbbbbbbbbbb");  
  Serial.println("line3: ccccccccccccccccccccccccc");  
  Serial.println("line4: ddddddddddddddddddddddddd");
  // delay to show that there will be no timeout on the client side; 
  // the client really waits FOREVER for the EOD character   
  delay(3000);   
  Serial.println("line5: aaaaaaaaaaaaaaaaaaaaaaaaa");  
  Serial.println("line6: bbbbbbbbbbbbbbbbbbbbbbbbb");  
  Serial.println("line7: ccccccccccccccccccccccccc");  
  Serial.println("line8: ddddddddddddddddddddddddd");    
  // append EOD !  
  Serial.print("#");    
  Serial.flush();
}

// Server on Standby
// =================
void loop() {
  
  // blink forever
  // -------------
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on 
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off 
  delay(1000);                       // wait for a second
}
