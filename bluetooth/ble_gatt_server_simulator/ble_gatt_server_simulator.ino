/*
  The purpose of this is to simulate a Bluetooth LE GATT server that would be used on an IoT device with sensors. 
  This decouples the design work for the Bluetooth interface from the data collection and processing code. It allows for 
  benchmarking and stress tests of data rates and latency.

This service generates a ramp sequency which currently counts up by 1 and is unsigned 8 bit

Service: RampGeneratorServiceU8
b3f8665e-9514-11ed-9f96-37eb16895c5e


    
  The software for interfacing with this device uses BlueR and can be found at:

  This has only been tested on the Arduino MKR WiFi 1010 using bluetooth LE commands.
 
 
  This example code is in the public domain.  
*/

#include <ArduinoBLE.h>
#include <utility/GAP.h>
//b3f8665e-9514-11ed-9f96-37eb16895c00 reserved as a service of information about all RampGenerator01ServiceU8
BLEService Ramp01ServiceByte("b3f8665e-9514-11ed-9f96-37eb16895c01");
BLEService Ramp02ServiceByte("b3f8665e-9514-11ed-9f96-37eb16895c02");

//
BLEByteCharacteristic Ramp01MinimumValue("b5720d32-9514-11ed-985d-7300cdba6b00", BLERead | BLEWrite);
BLEByteCharacteristic Ramp01MaximumValue("b5720d32-9514-11ed-985d-7300cdba6b01", BLERead | BLEWrite);
BLEByteCharacteristic Ramp01CurrentValue("b5720d32-9514-11ed-985d-7300cdba6b02", BLERead );
BLEByteCharacteristic Ramp01Start("b5720d32-9514-11ed-985d-7300cdba6b03", BLEWrite);
BLEByteCharacteristic Ramp01Stop("b5720d32-9514-11ed-985d-7300cdba6b04", BLEWrite);
BLEByteCharacteristic Ramp01Reset("b5720d32-9514-11ed-985d-7300cdba6b05", BLEWrite);

BLEByteCharacteristic Ramp02MinimumValue("b5720d32-9514-11ed-985d-7300cdba6b00", BLERead | BLEWrite);
BLEByteCharacteristic Ramp02MaximumValue("b5720d32-9514-11ed-985d-7300cdba6b01", BLERead | BLEWrite);
BLEByteCharacteristic Ramp02CurrentValue("b5720d32-9514-11ed-985d-7300cdba6b02", BLERead );
BLEByteCharacteristic Ramp02Start("b5720d32-9514-11ed-985d-7300cdba6b03", BLEWrite);
BLEByteCharacteristic Ramp02Stop("b5720d32-9514-11ed-985d-7300cdba6b04", BLEWrite);
BLEByteCharacteristic Ramp02Reset("b5720d32-9514-11ed-985d-7300cdba6b05", BLEWrite);



void setup() {
Serial.begin(115200);
while (!Serial);

pinMode(LED_BUILTIN, OUTPUT);
if (!BLE.begin()) 
{
Serial.println("starting BLE failed!");
while (1);
}

BLE.setLocalName("GATT_Server");

Ramp01ServiceByte.addCharacteristic(Ramp01MinimumValue);
Ramp01ServiceByte.addCharacteristic(Ramp01MaximumValue);
Ramp01ServiceByte.addCharacteristic(Ramp01CurrentValue);
Ramp01ServiceByte.addCharacteristic(Ramp01Start);
Ramp01ServiceByte.addCharacteristic(Ramp01Stop);
Ramp01ServiceByte.addCharacteristic(Ramp01Reset);
BLE.addService(Ramp01ServiceByte);

Ramp02ServiceByte.addCharacteristic(Ramp02MinimumValue);
Ramp02ServiceByte.addCharacteristic(Ramp02MaximumValue);
Ramp02ServiceByte.addCharacteristic(Ramp02CurrentValue);
Ramp02ServiceByte.addCharacteristic(Ramp02Start);
Ramp02ServiceByte.addCharacteristic(Ramp02Stop);
Ramp02ServiceByte.addCharacteristic(Ramp02Reset);
BLE.addService(Ramp02ServiceByte);


BLE.setAdvertisedService(Ramp01ServiceByte);
// 1600*0.625 msec = 1 sec
GAP.setAdvertisingInterval(1600);

BLE.advertise();
Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() 
{
BLEDevice central = BLE.central();

if (central) 
{
Serial.print("Connected to central: ");
Serial.println(central.address());
digitalWrite(LED_BUILTIN, HIGH);
Ramp01MinimumValue.writeValue(1);
Ramp01MaximumValue.writeValue(253);
Ramp01CurrentValue.writeValue(Ramp01MinimumValue.value());
while (central.connected()) {
     
      Ramp01CurrentValue.writeValue(Ramp01CurrentValue.value()+1);
      Serial.print("Ramp01 value is : ");
      Serial.print(Ramp01CurrentValue.value());
      delay(1000);
}
}
digitalWrite(LED_BUILTIN, LOW);
Serial.print("Disconnected from central: ");
Serial.println(central.address());
}