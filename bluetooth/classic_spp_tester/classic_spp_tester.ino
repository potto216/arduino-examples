/*
  The purpose of this is to provide a way to configure various Bluetooth Classic devices that support send direct HCI commands to the Bluetooth module connected to the processor.

  This has only been tested on the Arduino MKR WiFi 1010 using bluetooth LE commands.

  data sent at regular rate
  data read at regular rate
  data simulate
  ACL channel control
  See readme.md for examples. 

  This example code is in the public domain.
*/

#define SPP_MODULE_CONFIGURE_BAUD_RATE
#define SPP_MODULE_CONFIGURE_BAUD_RATE

  //Serial1.begin(9600); // works for default connect comm
  //Serial1.begin(38400); // works for default connect comm


void setup() {
  
  pinMode(0, OUTPUT);
  digitalWrite(0, LOW); //If connected then High works to get data at 9600bps also the baud rate on the other system doesn't matter. When you are connected you can type whatever and it will show up either ned

  //If connected then High works to get data at 9600bps also the baud rate on the other system doesn't matter. When you are connected you can type whatever and it will show up either ned
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH); 


  Serial.begin(230400);
  Serial.print("Enter AT Commands:");
  

  Serial1.begin(921600); // works for default connect comm
}

void loop() {  
  /*
  digitalWrite(0, HIGH);
  Serial1.write("High"); 
  Serial.write("High"); 
  delay(1000);

  Serial1.write("Low"); 
  Serial.write("Low"); 
  digitalWrite(0, LOW);
  delay(1000);
*/

/* Note: When sending AT commands, you must enter a carriage return and line feed. AT commands can only take effect when the module is not connected. Once the blue
The Bluetooth module is connected to the device, and the Bluetooth module enters the data transparent transmission mode)
Command details
(AT commands are case-sensitive and end with carriage return and line feed characters: \r\n) 

https://forum.arduino.cc/t/things-i-learned-about-hc-05-and-arduino/881035/9
*/

  // The code below allows for commands and messages to be sent from COMPUTER (serial monitor) -> HC-05
  if (Serial.available()) // Keep reading from Arduino Serial Monitor
  { 
    char ch = Serial.read();
    if(ch == 10)
    {
    Serial1.write(13);  
    Serial1.write(10);
    }
    else
    {
      Serial1.write(ch);
    }
  }



  // 
  if (Serial1.available()) // Keep reading from HC-05 and send to Arduino
  { 
    Serial.write(Serial1.read()); // Serial Monitor
  }

 //delay (1000);
 //Serial1.write("AT");
 //Serial1.write(13);
 //Serial1.write(10);
 /*
 When entering AT mode , some commands known as ‘Standard protocol’, ie..AT+UART, AT+URT? or AT+NAME? will not bring back a response. The only commands that brought back a response was AT+NAME, AT+BAUD, AT+ROLE, AT+ADDR, AT+PIN. I was able to change one device name typing AT+NAME[EXAMPLE] then the request to verify confirmed EXAMPLE. If I typed ie… AT+NAME=[EXAMPLE], it would confirm name was ‘EXAMPLE’; Expected to see one board with a ROLE value = 1. Both are 0.
Zero information on company website to address programming protocols. Have not gotten far enough to discover I they really talk to each other as both ADDR are different.

The bind command does not work. The default buad rate is 4800, not 38400. The set buad rate command does not work unless you add an extra comma to the end of your statement. The bind command flat out does not work. The RX and TX keys appear to be wired backwards. AT commands won't work unless the RX and TX pins are wired the wrong way (RX to RX and TX to TX). I cannot find anything to tell me what any of the five or six observed blinking patterns mean.

*/


}
