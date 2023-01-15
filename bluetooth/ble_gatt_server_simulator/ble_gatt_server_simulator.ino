/*
  The purpose of this is to simulate a Bluetooth LE GATT server that would be used on an IoT device with sensors. 
  This decouples the design work for the Bluetooth interface from the data collection and processing code. It allows for 
  benchmarking and stress tests of data rates and latency.

  The simulated sensors 
    
  The software for interfacing with this device uses BlueR and can be found at:

  This has only been tested on the Arduino MKR WiFi 1010 using bluetooth LE commands.
 
  This example code is in the public domain.  
*/

#include <ArduinoBLE.h>
#include <utility/GAP.h>

BLEService batteryService("1101");
BLEUnsignedCharCharacteristic batteryLevelChar("2101", BLERead | BLENotify);

void setup() {
Serial.begin(115200);
while (!Serial);

pinMode(LED_BUILTIN, OUTPUT);
if (!BLE.begin()) 
{
Serial.println("starting BLE failed!");
while (1);
}

BLE.setLocalName("BatteryMonitor");
BLE.setAdvertisedService(batteryService);
batteryService.addCharacteristic(batteryLevelChar);
BLE.addService(batteryService);

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

while (central.connected()) {

      int battery = analogRead(A0);
      int batteryLevel = map(battery, 0, 1023, 0, 100);
      Serial.print("Battery Level % is now: ");
      Serial.println(batteryLevel);
      batteryLevelChar.writeValue(batteryLevel);
      delay(200);

}
}
digitalWrite(LED_BUILTIN, LOW);
Serial.print("Disconnected from central: ");
Serial.println(central.address());
}