/*
  The purpose of this is to simulate a Bluetooth LE GATT server that would be used on an IoT device with sensors. 
  This decouples the design work for the Bluetooth interface from the data collection and processing code. It allows for 
  benchmarking and stress tests of data rates and latency.

  This service generates a stream of test data which currently counts up by 1 and is unsigned 8 bit

  Service: StreamService
  3a2d2e61-e54f-4fc1-9dc4-15a532c18c5e

  The software for interfacing with this device uses BlueR and can be found at:

  This has only been tested on the Arduino MKR WiFi 1010 using bluetooth LE commands.
 
  This example code is in the public domain.  
*/

#include <ArduinoBLE.h>
#include <utility/GAP.h>
#include <utility/HCI.h>

// 3a2d2e61-e54f-4fc1-9dc4-15a532c18c00 reserved as a service of information about all AclStreamService
BLEService AclStreamService("3a2d2e61-e54f-4fc1-9dc4-15a532c18c01");

static constexpr byte STREAM_COMMAND_STOP = 0;
static constexpr byte STREAM_COMMAND_START = 1;
static constexpr byte STREAM_COMMAND_RESET = 2;

static constexpr byte STREAM_COMMAND_RESULT_SUCCESS = 0;
static constexpr byte STREAM_COMMAND_RESULT_NONE_RECEIVED = 1;
static constexpr byte STREAM_COMMAND_RESULT_ERROR = 2;

static constexpr byte STREAM_STATUS_STOPPED = 0;
static constexpr byte STREAM_STATUS_RUNNING = 1;

static constexpr byte SEND_PACKETS_FOREVER = 0;
static constexpr byte DEFAULT_CID = 0x78;

static constexpr uint8_t  DEFAULT_TX_OCTETS =  251; // (0xFB) maximum number of 251 payload octets
static constexpr uint16_t DEFAULT_TX_TIME = 0x0848; // 2,120 microseconds

static constexpr uint8_t  HCI_PARM_START_IDX = 3;
static constexpr uint8_t HCI_LENGTH_BYTE_IDX = 2;

BLEByteCharacteristic AclStreamCommand("5138f1ac-bf01-42fc-a321-09afd4de6e01", BLEWrite);
BLEByteCharacteristic AclStreamCommandStatus("5138f1ac-bf01-42fc-a321-09afd4de6e02", BLERead | BLENotify );
BLEByteCharacteristic AclStreamStatus("5138f1ac-bf01-42fc-a321-09afd4de6e03", BLERead | BLENotify );

//
BLEByteCharacteristic AclStream_CID("5138f1ac-bf01-42fc-a321-09afd4de6e05", BLERead);
BLEUnsignedShortCharacteristic AclStream_Connection_Handle("5138f1ac-bf01-42fc-a321-09afd4de6e06", BLERead);

// TX_Octets (2 octets): Preferred maximum number of payload octets that the local Controller should include in a single LL Data PDU on this connection. ex: 251 (0xFB) 
BLEUnsignedShortCharacteristic AclStream_TX_Octets("5138f1ac-bf01-42fc-a321-09afd4de6e07", BLERead | BLEWrite);

// TX_Time (2 octets): Preferred maximum number of microseconds that the local Controller should use to transmit a single Link Layer packet containing an LL Data PDU on this connection. ex: 2,120 microseconds (0x0848). 
BLEUnsignedShortCharacteristic AclStream_TX_Time("5138f1ac-bf01-42fc-a321-09afd4de6e08", BLERead | BLEWrite);

BLEUnsignedLongCharacteristic AclStreamTotalPacketsToSend("5138f1ac-bf01-42fc-a321-09afd4de6e09", BLERead | BLEWrite);
BLEUnsignedLongCharacteristic AclStreamTotalPacketsSent("5138f1ac-bf01-42fc-a321-09afd4de6e0a", BLERead | BLEWrite);

void setup() {
Serial.begin(115200);

// If running off battery without the USB cable connected the following line should be commented out
while (!Serial);  

pinMode(LED_BUILTIN, OUTPUT);
if (!BLE.begin()) 
{
  Serial.println("starting BLE failed!");
  while (1);
}

BLE.setLocalName("ACL_Data_Streamer");

AclStreamService.addCharacteristic(AclStreamCommand);
AclStreamService.addCharacteristic(AclStreamCommandStatus);
AclStreamService.addCharacteristic(AclStream_CID);
AclStreamService.addCharacteristic(AclStream_Connection_Handle);
AclStreamService.addCharacteristic(AclStream_TX_Octets);
AclStreamService.addCharacteristic(AclStream_TX_Time);
AclStreamService.addCharacteristic(AclStreamTotalPacketsToSend);
AclStreamService.addCharacteristic(AclStreamTotalPacketsSent);


BLE.addService(AclStreamService);

BLE.setAdvertisedService(AclStreamService);
// 1600*0.625 msec = 1 sec
GAP.setAdvertisingInterval(1600);

BLE.advertise();

Serial.print("Bluetooth device ");
Serial.print(BLE.address());
Serial.println(" advertising and waiting for connections...");

}

void loop() 
{
BLEDevice central = BLE.central();

uint8_t packetData[]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
                      0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
                      0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
                      0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
                      0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
                      0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f,
                      0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
                      0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
                      0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
                      0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
                      0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAa,0xAb,0xAc,0xAd,0xAe,0xAf,
                      0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBa,0xBb,0xBc,0xBd,0xBe,0xBf,
                      0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCa,0xCb,0xCc,0xCd,0xCe,0xCf,
                      0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDa,0xDb,0xDc,0xDd,0xDe,0xDf,
                      0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEa,0xEb,0xEc,0xEd,0xEe,0xEf,                         
                        };
uint16_t packetDataArraySize=sizeof(packetData);
//                   index    0     1     2     3     4     5     6     7     8
uint8_t hci_send_buffer[]={0x22, 0x20, 0x06, 0x00, 0x00, 0xFB, 0x00, 0x48, 0x08};
const uint8_t TX_OCTETS_IDX_LSB = 5;
const uint8_t TX_OCTETS_IDX_MSB = 6;
const uint8_t TX_TIME_IDX_LSB   = 7;
const uint8_t TX_TIME_IDX_MSB   = 8;

const int hci_send_buffer_free_index=sizeof(hci_send_buffer);
const uint16_t opcode=(hci_send_buffer[1]<<8 | hci_send_buffer[0]);
const uint8_t hciPacketLength = hci_send_buffer_free_index - HCI_PARM_START_IDX;
int result;


if (central) 
{  
  BLE.stopAdvertise();
  Serial.print("Connected to central: ");
  Serial.println(central.address());

  uint16_t connectionHandle=HCI.connectionHandle;
  uint8_t cid=DEFAULT_CID;
  uint32_t totalPacketsToSend=0;
  uint32_t totalPacketsSent=0;
  uint16_t TX_Octets=0;
  uint16_t TX_Time=0;  

  AclStreamCommandStatus.writeValue(STREAM_COMMAND_RESULT_NONE_RECEIVED); 
  AclStreamStatus.writeValue(STREAM_STATUS_STOPPED);
  AclStream_CID.writeValue(cid);

  AclStream_Connection_Handle.writeValue(connectionHandle);
  AclStream_TX_Octets.writeValue(DEFAULT_TX_OCTETS);
  AclStream_TX_Time.writeValue(DEFAULT_TX_TIME);
  AclStreamTotalPacketsToSend.writeValue(totalPacketsToSend);
  AclStreamTotalPacketsSent.writeValue(totalPacketsSent);

  PinStatus ledState=HIGH;
  digitalWrite(LED_BUILTIN, ledState);
  while (central.connected())
  {   
    if (AclStreamCommand.written())
    {
      byte command = AclStreamCommand.value();
      switch(command)
      {
        case STREAM_COMMAND_STOP:
          Serial.println("STREAM_COMMAND_STOP received.");
          AclStreamStatus.writeValue(STREAM_STATUS_STOPPED);
          AclStreamCommandStatus.writeValue(STREAM_COMMAND_RESULT_SUCCESS);
          break;

        case STREAM_COMMAND_START:
          Serial.println("STREAM_COMMAND_START received.");
          totalPacketsToSend = AclStreamTotalPacketsToSend.value();
          TX_Octets = AclStream_TX_Octets.value();
          TX_Time = AclStream_TX_Time.value();
          Serial.print("totalPacketsToSend = ");
          Serial.println(totalPacketsToSend);

          /**********************************************************
          HCI_LE_Set_Data_Length 0x0022 Ref: BLUETOOTH CORE SPECIFICATION Version 5.3 | Vol 4, Part E page 2415
          For the LE Controller commands, the OGF code is defined as 0x08.

          HCI_LE_Set_Data_Length OCF+OGF<<2: 0x2022
          |  
          |     Length: 6 octets
          |     |  Connection_Handle: 0x0000
          |     |  |     TX_Octets: 251 (0xFB) Preferred maximum number of payload octets that the local Controller should include in a single LL Data PDU on this connection.
          |     |  |     |     TX_Time: 2,120 microseconds (0x0848). Preferred maximum number of microseconds that the local Controller should use to transmit a single Link Layer packet containing an LL Data PDU on this connection.
          |     |  |     |     | 
          |     |  |     |     | 
          |     |  |     |     | 
          |     |  |     |     | 
          22 20 06 00 00 FB 00 48 08
          ********************************************************/
          //                     index   0      1     2     3     4     5     6     7     8
          // uint8_t hci_send_buffer[]={0x22, 0x20, 0x06, 0x00, 0x00, 0xFB, 0x00, 0x48, 0x08};

          hci_send_buffer[TX_OCTETS_IDX_LSB] = (TX_Octets && 0xFF);
          hci_send_buffer[TX_OCTETS_IDX_MSB] = ((TX_Octets>>8) && 0xFF);
          hci_send_buffer[TX_TIME_IDX_LSB]   = (TX_Time && 0xFF);
          hci_send_buffer[TX_TIME_IDX_MSB]   = ((TX_Time>>8) && 0xFF);
          
          result = HCI.sendCommand(opcode, hciPacketLength, &hci_send_buffer[HCI_PARM_START_IDX]);
          BLE.poll();

          Serial.println("Sending data packets");
          AclStreamStatus.writeValue(STREAM_STATUS_RUNNING);
          AclStreamCommandStatus.writeValue(STREAM_COMMAND_RESULT_SUCCESS);
          break;

        case STREAM_COMMAND_RESET:
          Serial.println("STREAM_COMMAND_RESET received.");
          AclStream_TX_Octets.writeValue(DEFAULT_TX_OCTETS);
          AclStream_TX_Time.writeValue(DEFAULT_TX_TIME);
          totalPacketsToSend=0;
          totalPacketsSent=0;  

          for(int ii=0; ii<packetDataArraySize; ii++)
          {
            packetData[ii]=ii;
          } 

          AclStreamTotalPacketsToSend.writeValue(totalPacketsToSend);
          AclStreamTotalPacketsSent.writeValue(totalPacketsSent);

          AclStreamStatus.writeValue(STREAM_STATUS_STOPPED);
          AclStreamCommandStatus.writeValue(STREAM_COMMAND_RESULT_SUCCESS);
          break;

        default:
          Serial.print("Unsupported command received of 0x");
          Serial.println(command, HEX);
          AclStreamCommandStatus.writeValue(STREAM_COMMAND_RESULT_ERROR);

      }

    }

    if (AclStreamStatus.value() == STREAM_STATUS_RUNNING )
    {
      if (totalPacketsSent==SEND_PACKETS_FOREVER || totalPacketsSent<totalPacketsToSend)
      {

        // Keep the array offset consistent by updating all the elements
        for(int ii=0; ii<packetDataArraySize; ii++)
        {
          packetData[ii]=packetData[ii]+1;
        }        
        HCI.sendAclPkt(connectionHandle, cid, TX_Octets, packetData);        
        BLE.poll();

        totalPacketsSent++;
        AclStreamTotalPacketsSent.writeValue(totalPacketsSent);
      }


    }

  BLE.setAdvertisedService(AclStreamService);
  // 1600*0.625 msec = 1 sec
  GAP.setAdvertisingInterval(1600);

  BLE.advertise();
  }
}
digitalWrite(LED_BUILTIN, LOW);
//Serial.print("Disconnected from central: ");
//Serial.println(central.address());
}