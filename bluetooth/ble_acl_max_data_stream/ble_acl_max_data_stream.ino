/*
  The purpose of this is to test how fast data can be streamed from the Bluetooth module to a recieving device. 
  The device acts as a central and scans for a predetermined perpherial to connect to and stat sending test packets.
  Throughput is updated on the devie.

  
  The software for receiving uses BlueR and can be found at:

  This has only been tested on the Arduino MKR WiFi 1010 using bluetooth LE commands.

  Troubleshooting
  1. Verify the bluetooth address is correct
  2. Verify it is in the correct order
  
  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <utility/HCI.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("Bluetooth® Low Energy Central - ble_acl_max_data_stream version 0.0.1");
  
  // start scanning for peripherals
  BLE.scan();
}

// if the number is 0 then it will send an unlimited amount
#define TOTAL_PACKETS_TO_SEND 10

void loop() {

  //This is the Arduino BD Address: 58:bf:25:9c:50:7e
  //uint8_t centralAddress[6]={0x7e, 0x50, 0x9c, 0x25, 0xbf, 0x58 };
  
  //uint8_t peripheralAddressType=0;
  // this is the Bluetooth address of the device you wish to connect to.
  //	BD Address: 00:e0:42:ab:3d:03
  uint8_t peripheralAddress[6]={0x03, 0x3D, 0xAB, 0x42, 0xE0, 0x00 };
  //uint8_t myBdaddrType=0;
   



  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    if (peripheral.hasAddress(peripheralAddress,peripheralAddress)) {      
      Serial.print("Found our device: ");
      Serial.print(peripheral.address());
      Serial.print(" '");
      Serial.print(peripheral.localName());
      Serial.println("'. NOT ours.");      
      
      BLE.stopScan();
    
      
      if (peripheral.connect()) {
        Serial.println("Connected");
      } else {
        Serial.println("Failed to connect!");
        return;
      }

/* Handle info
0xXXXX The Handle for the connection for which the RSSI is to be read.
The Handle is a Connection_Handle for an ACL-U or LE-U connection.
Range: 0x0000 to 0x0EFF
HCI_Read_RSSI

Event(s) generated (unless masked away):
When the Controller receives the HCI_LE_Create_Connection command, the
Controller sends the HCI_Command_Status event to the Host. An HCI_LE_-
Connection_Complete or HCI_LE_Enhanced_Connection_Complete event
shall be generated when a connection is created because of this command or
the connection creation procedure is cancelled; until one of these events is
generated, the command is considered pending. If a connection is created and
the Controller supports the LE Channel Selection Algorithm #2 feature, this
event shall be immediately followed by an HCI_LE_Channel_Selection_-
Algorithm event.

The HCI_Connection_Complete event indicates to both of the Hosts forming
the connection that a new connection has been established. This event also
indicates to the Host which issued the HCI_Create_Connection, HCI_Accept_-
Connection_Request, or HCI_Reject_Connection_Request command, and
then received an HCI_Command_Status event, if the issued command failed or
was successful.

*/

    uint16_t connectionHandle=HCI.connection_handle;
    uint8_t cid=0x78;

    Serial.print("HCI.connection_handle:");
    Serial.println(HCI.connection_handle);

      #define HCI_PARM_START_IDX 3 
      #define HCI_LENGTH_BYTE_IDX 2 

      uint8_t hci_send_buffer[]={0x22, 0x20, 0x06, 0x00, 0x00, 0xFB, 0x00, 0x48, 0x08};
      
      int hci_send_buffer_free_index=sizeof(hci_send_buffer);
      uint16_t opcode=(hci_send_buffer[1]<<8 | hci_send_buffer[0]);
      uint8_t packetLength = hci_send_buffer_free_index - HCI_PARM_START_IDX;
      int result = HCI.sendCommand(opcode, packetLength, &hci_send_buffer[HCI_PARM_START_IDX]);
    
      Serial.println("Sending data packets");

      //0x00-0x15 is what worked before
      uint8_t packetData[]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
                            0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
                            0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
                            0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
                            0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
                            0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f };                                                                                    
      uint8_t packetLength=sizeof(packetData);
      uint8_t packetsSent=0;
      while (packetsSent < TOTAL_PACKETS_TO_SEND) {




        for(int i=0; i<packetLength; i++)
        {
          packetData[i]=packetData[i]+1;
        }

        BLE.poll();
        HCI.sendAclPkt(connectionHandle, cid, packetLength, packetData);

          //HCI.sendAclPkt_debug(connectionHandle, cid, packetLength, packetData);
        packetsSent++;
      }

      peripheral.disconnect();
      BLE.poll();
      while(1);  // hangout here forever.
    }
    else
    {
      Serial.print("Found a device: ");
      Serial.print(peripheral.address());
      Serial.print(" '");
      Serial.print(peripheral.localName());
      Serial.println("'. NOT ours.");
      
    }
  }
}

/*
void explorerPeripheral(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
//    peripheral.disconnect();
    return;
  }

  // read and print device name of peripheral
  Serial.println();
  Serial.print("Device name: ");
  Serial.println(peripheral.deviceName());
  Serial.print("Appearance: 0x");
  Serial.println(peripheral.appearance(), HEX);
  Serial.println();

  // loop the services of the peripheral and explore each
  for (int i = 0; i < peripheral.serviceCount(); i++) {
    BLEService service = peripheral.service(i);

    exploreService(service);
  }

  Serial.println();

  // we are done exploring, disconnect
  Serial.println("Disconnecting ...");
//  peripheral.disconnect();
  Serial.println("Disconnected");
}

void exploreService(BLEService service) {
  // print the UUID of the service
  Serial.print("Service ");
  Serial.println(service.uuid());

  // loop the characteristics of the service and explore each
  for (int i = 0; i < service.characteristicCount(); i++) {
    BLECharacteristic characteristic = service.characteristic(i);

    exploreCharacteristic(characteristic);
  }
}

void exploreCharacteristic(BLECharacteristic characteristic) {
  // print the UUID and properties of the characteristic
  Serial.print("\tCharacteristic ");
  Serial.print(characteristic.uuid());
  Serial.print(", properties 0x");
  Serial.print(characteristic.properties(), HEX);

  // check if the characteristic is readable
  if (characteristic.canRead()) {
    // read the characteristic value
    characteristic.read();

    if (characteristic.valueLength() > 0) {
      // print out the value of the characteristic
      Serial.print(", value 0x");
      printData(characteristic.value(), characteristic.valueLength());
    }
  }
  Serial.println();

  // loop the descriptors of the characteristic and explore each
  for (int i = 0; i < characteristic.descriptorCount(); i++) {
    BLEDescriptor descriptor = characteristic.descriptor(i);

    exploreDescriptor(descriptor);
  }
}

void exploreDescriptor(BLEDescriptor descriptor) {
  // print the UUID of the descriptor
  Serial.print("\t\tDescriptor ");
  Serial.print(descriptor.uuid());

  // read the descriptor value
  descriptor.read();

  // print out the value of the descriptor
  Serial.print(", value 0x");
  printData(descriptor.value(), descriptor.valueLength());

  Serial.println();
}

void printData(const unsigned char data[], int length) {
  for (int i = 0; i < length; i++) {
    unsigned char b = data[i];

    if (b < 16) {
      Serial.print("0");
    }

    Serial.print(b, HEX);
  }
}
*/