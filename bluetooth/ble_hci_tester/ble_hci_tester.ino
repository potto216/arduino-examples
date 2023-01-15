/*
  The purpose of this is to send direct HCI commands to the Bluetooth module connected to the processor.

  This has only been tested on the Arduino MKR WiFi 1010 using bluetooth LE commands.

  See readme.md for examples. 

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include "utility/HCI.h"

// Assumes the ASCII character set!!
#define ST_WAITING_FOR_COMMAND 0
#define ST_WAITING_FOR_HEX_MSN_OR_END_HCI_COMMAND 1
#define ST_WAITING_FOR_HEX_LSN 2

#define MAX_HCI_SEND_BUFFER_SZ 255
uint8_t hci_send_buffer[MAX_HCI_SEND_BUFFER_SZ];
int hci_send_buffer_free_index=0;

#define MAX_HCI_RECEIVE_BUFFER_SZ 255
uint8_t hci_receive_buffer[MAX_HCI_RECEIVE_BUFFER_SZ];
int hci_receive_buffer_used_length=0;
int current_state = ST_WAITING_FOR_COMMAND;
int debug_mode=true;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  HCI.debug(Serial);
  
  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  BLE.scan();
  BLE.stopScan();
  Serial.println("Bluetooth® Low Energy Central - HCI tester");   
  Serial.println("Ready for HCI Commands (version 0.1.0)");
  current_state = ST_WAITING_FOR_COMMAND;
  debug_mode=false;
}

void loop() {

 if (Serial.available()) // Keep reading from Arduino Serial Monitor
  { 
    char ch = Serial.read();
    switch(current_state)
    {
      case ST_WAITING_FOR_COMMAND:
        if(debug_mode) { Serial.println("[ST_WAITING_FOR_COMMAND]"); }
        switch(ch)
        {      
          case 'P': // ping command
          case 'p':
            Serial.println("Ping received");
            current_state = ST_WAITING_FOR_COMMAND;
            break;

          case 'D': // Debug mode
          case 'd':
            if(debug_mode)
            {
              Serial.println("debug_mode set to off");
              debug_mode = false;
            }
            else
            {
              Serial.println("debug_mode set to on");
              debug_mode = true;
            }
            current_state = ST_WAITING_FOR_COMMAND;
            break;
          
          case 'H': //start of HCI command
            hci_send_buffer_free_index=0;
            Serial.print("H");
            current_state = ST_WAITING_FOR_HEX_MSN_OR_END_HCI_COMMAND;            
            break;
          case 10: //ignore newline and carrage return
          case 13:
            current_state = ST_WAITING_FOR_COMMAND;
            break;  
          default:
            Serial.print("Unknown command:");
            Serial.print(ch);
            Serial.print("(char in hex): ");
            Serial.println(ch, HEX);
            current_state = ST_WAITING_FOR_COMMAND;
        }        
        break;

      case ST_WAITING_FOR_HEX_MSN_OR_END_HCI_COMMAND:
        if(debug_mode) { Serial.println("[ST_WAITING_FOR_HEX_MSN_OR_END_HCI_COMMAND]"); }
        if(ch=='h') //end of data, now send the data and return the result
        {

          #define HCI_PARM_START_IDX 3 
          #define HCI_LENGTH_BYTE_IDX 2 
          if((hci_send_buffer_free_index >= HCI_LENGTH_BYTE_IDX) && (hci_send_buffer_free_index< MAX_HCI_SEND_BUFFER_SZ))
          {
            uint16_t opcode=(hci_send_buffer[1]<<8 | hci_send_buffer[0]);
            uint8_t plen = hci_send_buffer_free_index - HCI_PARM_START_IDX;
            if (plen != hci_send_buffer[HCI_LENGTH_BYTE_IDX])
            {
                Serial.print("Error. Expected length to match parameter length. It was ");
                Serial.print(hci_send_buffer[HCI_LENGTH_BYTE_IDX]);
                Serial.print("And not the expected ");
                Serial.println(plen);
                hci_send_buffer_free_index = 0;
                current_state = ST_WAITING_FOR_COMMAND;
                break;
            }


            int result = HCI.sendCommand(opcode, plen, &hci_send_buffer[HCI_PARM_START_IDX]);
            if (result == 0)             
            {
              if (HCI._cmdResponseLen > MAX_HCI_RECEIVE_BUFFER_SZ)
              {
                Serial.print("Error! HCI._cmdResponseLen of ");
                Serial.print(HCI._cmdResponseLen);
                Serial.println("is invalid.");
                current_state = ST_WAITING_FOR_COMMAND;
                break;
              }
              else
              {
                hci_receive_buffer_used_length=HCI._cmdResponseLen;
                memcpy(hci_receive_buffer, HCI._cmdResponse, HCI._cmdResponseLen);
                Serial.print("Received:[");
                for(int ii=0; ii<hci_receive_buffer_used_length; ii++ )
                {
                  Serial.print(hci_receive_buffer[ii], HEX);
                  if(ii!=(hci_receive_buffer_used_length-1))
                  {
                    Serial.print(", ");
                  }                  
                }
                Serial.println("]");
                current_state = ST_WAITING_FOR_COMMAND;
                break;
              }
              
            }
            else
            {
                Serial.print("ERROR: Wanted an HCI result of 0 but got ");
                Serial.println(result, HEX);
                hci_receive_buffer_used_length=HCI._cmdResponseLen;
                memcpy(hci_receive_buffer, HCI._cmdResponse, HCI._cmdResponseLen);
                Serial.print("Received:[");
                for(int ii=0; ii<hci_receive_buffer_used_length; ii++ )
                {
                  Serial.print(hci_receive_buffer[ii], HEX);
                  if(ii!=(hci_receive_buffer_used_length-1))
                  {
                    Serial.print(", ");
                  }                  
                }
                Serial.println("]");

                current_state = ST_WAITING_FOR_COMMAND;
                break;
            }
          }
          else
          {
            Serial.print("ERROR! hci_send_buffer_free_index is an invalid value of: ");
            Serial.println(hci_send_buffer_free_index, HEX);
            hci_send_buffer_free_index = 0;
            current_state = ST_WAITING_FOR_COMMAND;
            break;
          }
          // Done processing the command
          current_state = ST_WAITING_FOR_COMMAND;
          break;
        }

        //convert to upper case
        switch(ch)
        {
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
            ch = ch - ('a'-'A');
            break;          
          default:
          //do nothing
            break;
        }

        // Save hex char
        switch(ch)
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
            Serial.print(ch);
            if(hci_send_buffer_free_index>=0 && hci_send_buffer_free_index<MAX_HCI_SEND_BUFFER_SZ)
            {
              int value=-1;
              if(ch >='0' && ch <='9')
              {
                value = ch - '0';
              }
              if(ch >='A' && ch <='F')
              {
                value = ch - 'A' + 0xA;
              }
              if (value>=0 && value<=0xF)
              {
                hci_send_buffer[hci_send_buffer_free_index] = value << 4;
                current_state = ST_WAITING_FOR_HEX_LSN;
              }
              else
              {
                Serial.print("Value (");
                Serial.print(value);
                Serial.println(") is invalid. Reset state to ST_WAITING_FOR_COMMAND");
                current_state = ST_WAITING_FOR_COMMAND;
              }
              
            }
            break;

          default:
            Serial.print("Expected hex char, but received");
            Serial.print(ch);
            Serial.println(". Reset state to ST_WAITING_FOR_COMMAND");
            current_state = ST_WAITING_FOR_COMMAND;
            break;
        }
        break;

      case ST_WAITING_FOR_HEX_LSN:
        if(debug_mode) { Serial.println("[ST_WAITING_FOR_HEX_LSN]"); }
        switch(ch)
        {
          case 'a':
          case 'b':
          case 'c':
          case 'd':
          case 'e':
          case 'f':
            ch = ch - ('a'-'A');
            break;          
          default:
          //do nothing
            break;
        }
        switch(ch)
        {
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
          case 'A':
          case 'B':
          case 'C':
          case 'D':
          case 'E':
          case 'F':
            Serial.print(ch);
            if(hci_send_buffer_free_index>=0 && hci_send_buffer_free_index<MAX_HCI_SEND_BUFFER_SZ)
            {
              int value=-1;
              if(ch >='0' && ch <='9')
              {
                value = ch - '0';
              }
              if(ch >='A' && ch <='F')
              {
                value = ch - 'A' + 0xA;
              }
              if (value>=0 && value<=0xF)
              {
                hci_send_buffer[hci_send_buffer_free_index] = hci_send_buffer[hci_send_buffer_free_index] | value;
                hci_send_buffer_free_index++;
                current_state = ST_WAITING_FOR_HEX_MSN_OR_END_HCI_COMMAND;
              }
              else
              {
                Serial.print("Value (");
                Serial.print(value);
                Serial.println(") is invalid. Reset state to ST_WAITING_FOR_COMMAND");
                current_state = ST_WAITING_FOR_COMMAND;
              }
              
            }
            break;
          default:
            Serial.print("Expected hex char, but received");
            Serial.print(ch);
            Serial.println(". Reset state to ST_WAITING_FOR_COMMAND");
            current_state = ST_WAITING_FOR_COMMAND;
            break;
        }
        break;        



      default:
        Serial.println("Error bad state!");
        current_state = ST_WAITING_FOR_COMMAND;
        break;
    }

  }


}
