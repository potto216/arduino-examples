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

void do_nothing(void);
//b3f8665e-9514-11ed-9f96-37eb16895c00 reserved as a service of information about all RampGenerator01ServiceU8
BLEService Ramp01ServiceByte("b3f8665e-9514-11ed-9f96-37eb16895c01");
BLEService Ramp02ServiceByte("b3f8665e-9514-11ed-9f96-37eb16895c02");

static constexpr byte RAMP_COMMAND_STOP = 0;
static constexpr byte RAMP_COMMAND_START = 1;
static constexpr byte RAMP_COMMAND_RESET = 2;
static constexpr byte RAMP_COMMAND_TEST_IO = 3;

static constexpr byte RAMP_COMMAND_RESULT_SUCCESS = 0;
static constexpr byte RAMP_COMMAND_RESULT_NONE_RECEIVED = 1;
static constexpr byte RAMP_COMMAND_RESULT_ERROR = 2;

static constexpr byte RAMP_STATUS_STOPPED = 0;
static constexpr byte RAMP_STATUS_RUNNING = 1;

static constexpr byte IO_TEST_PIN = 0;

//
BLEByteCharacteristic Ramp01MinimumValue("b5720d32-9514-11ed-985d-7300cdba6b00", BLERead | BLEWrite);
BLEByteCharacteristic Ramp01MaximumValue("b5720d32-9514-11ed-985d-7300cdba6b01", BLERead | BLEWrite);
BLEByteCharacteristic Ramp01CurrentValue("b5720d32-9514-11ed-985d-7300cdba6b02", BLERead | BLENotify);
BLEByteCharacteristic Ramp01Command("b5720d32-9514-11ed-985d-7300cdba6b03", BLEWrite);
BLEByteCharacteristic Ramp01CommandStatus("b5720d32-9514-11ed-985d-7300cdba6b04", BLERead | BLENotify);
BLEByteCharacteristic Ramp01Status("b5720d32-9514-11ed-985d-7300cdba6b05", BLERead | BLENotify);
BLEByteCharacteristic Ramp01StepTime("b5720d32-9514-11ed-985d-7300cdba6b06", BLERead | BLEWrite);


BLEByteCharacteristic Ramp02MinimumValue("b5720d32-9514-11ed-985d-7300cdba6b00", BLERead | BLEWrite);
BLEByteCharacteristic Ramp02MaximumValue("b5720d32-9514-11ed-985d-7300cdba6b01", BLERead | BLEWrite);
BLEByteCharacteristic Ramp02CurrentValue("b5720d32-9514-11ed-985d-7300cdba6b02", BLERead | BLENotify);
BLEByteCharacteristic Ramp02Command("b5720d32-9514-11ed-985d-7300cdba6b03", BLEWrite);
BLEByteCharacteristic Ramp02CommandStatus("b5720d32-9514-11ed-985d-7300cdba6b04", BLERead | BLENotify);
BLEByteCharacteristic Ramp02Status("b5720d32-9514-11ed-985d-7300cdba6b05", BLERead | BLENotify);
BLEByteCharacteristic Ramp02StepTime("b5720d32-9514-11ed-985d-7300cdba6b06", BLERead | BLEWrite);


void setup() {
Serial.begin(115200);

// If running off battery without the USB cable connected the following line should be commented out
while (!Serial);  

pinMode(LED_BUILTIN, OUTPUT);

pinMode(IO_TEST_PIN, OUTPUT);
digitalWrite(IO_TEST_PIN, LOW);

if (!BLE.begin()) 
{
  Serial.println("starting BLE failed!");
  while (1);
}

BLE.setLocalName("GATT_Server");

Ramp01ServiceByte.addCharacteristic(Ramp01MinimumValue);
Ramp01ServiceByte.addCharacteristic(Ramp01MaximumValue);
Ramp01ServiceByte.addCharacteristic(Ramp01CurrentValue);
Ramp01ServiceByte.addCharacteristic(Ramp01Command);
Ramp01ServiceByte.addCharacteristic(Ramp01CommandStatus);
Ramp01ServiceByte.addCharacteristic(Ramp01Status);
Ramp01ServiceByte.addCharacteristic(Ramp01StepTime);
BLE.addService(Ramp01ServiceByte);

Ramp02ServiceByte.addCharacteristic(Ramp02MinimumValue);
Ramp02ServiceByte.addCharacteristic(Ramp02MaximumValue);
Ramp02ServiceByte.addCharacteristic(Ramp02CurrentValue);
Ramp02ServiceByte.addCharacteristic(Ramp02Command);
Ramp02ServiceByte.addCharacteristic(Ramp02CommandStatus);
Ramp02ServiceByte.addCharacteristic(Ramp02Status);
Ramp02ServiceByte.addCharacteristic(Ramp02StepTime);
BLE.addService(Ramp02ServiceByte);

BLE.setAdvertisedService(Ramp01ServiceByte);
// 1600*0.625 msec = 1 sec
// 400*0.625 msec = 0.25 sec
GAP.setAdvertisingInterval(400);

BLE.advertise();
Serial.print("Bluetooth device ");
Serial.print(BLE.address());
Serial.println(" advertising and waiting for connections...");

}

void loop() 
{

BLEDevice central = BLE.central();

if (central) 
{
  Serial.print("Connected to central: ");
  Serial.println(central.address());

  Ramp01CommandStatus.writeValue(RAMP_COMMAND_RESULT_NONE_RECEIVED);
  Ramp01MinimumValue.writeValue(1);
  Ramp01MaximumValue.writeValue(253);
  Ramp01CurrentValue.writeValue(Ramp01MinimumValue.value());
  Ramp01Status.writeValue(RAMP_STATUS_STOPPED);
  Ramp01StepTime.writeValue(1);

  PinStatus ledState=HIGH;
  digitalWrite(LED_BUILTIN, ledState);
  while (central.connected()) 
  {   
    if (Ramp01Command.written())
    {
      byte command = Ramp01Command.value();
      switch(command)
      {
        case RAMP_COMMAND_STOP:
        Serial.println("RAMP_COMMAND_STOP received.");
          Ramp01Status.writeValue(RAMP_STATUS_STOPPED);
          Ramp01CommandStatus.writeValue(RAMP_COMMAND_RESULT_SUCCESS);
          break;

        case RAMP_COMMAND_START:
        Serial.println("RAMP_COMMAND_START received.");
          Ramp01Status.writeValue(RAMP_STATUS_RUNNING);
          Ramp01CommandStatus.writeValue(RAMP_COMMAND_RESULT_SUCCESS);
          break;

        case RAMP_COMMAND_RESET:
          Serial.println("RAMP_COMMAND_RESET received.");
          Ramp01MinimumValue.writeValue(1);
          Ramp01MaximumValue.writeValue(253);
          Ramp01CurrentValue.writeValue(Ramp01MinimumValue.value());
          Ramp01Status.writeValue(RAMP_STATUS_STOPPED);
          Ramp01StepTime.writeValue(1);
          Ramp01CommandStatus.writeValue(RAMP_COMMAND_RESULT_SUCCESS);
          break;
        
        case RAMP_COMMAND_TEST_IO:
          Serial.println("RAMP_COMMAND_TEST_IO received.");
          Ramp01CommandStatus.writeValue(RAMP_COMMAND_RESULT_SUCCESS);
          digitalWrite(IO_TEST_PIN, HIGH);
          for(int ii=1; ii<1000; ii++)    {  do_nothing(); }
          digitalWrite(IO_TEST_PIN, LOW);
          for(int ii=1; ii<1000; ii++)    { do_nothing();    }
          digitalWrite(IO_TEST_PIN, HIGH);
          for(int ii=1; ii<1000; ii++)    { do_nothing();    }
          digitalWrite(IO_TEST_PIN, LOW);          
          break;

        default:
          Serial.print("Unsupported command received of 0x");
          Serial.println(command, HEX);
          Ramp01CommandStatus.writeValue(RAMP_COMMAND_RESULT_ERROR);

      }

    }

    if (Ramp01Status.value() == RAMP_STATUS_RUNNING )
    {
      Serial.print("Ramp01 value is ");
      Serial.println(Ramp01CurrentValue.value());
      delay(Ramp01StepTime.value()*1000); 
      if(Ramp01CurrentValue.value()>=Ramp01MaximumValue.value())
      {
        Ramp01CurrentValue.writeValue(Ramp01MinimumValue.value());
      }
      else
      {
        Ramp01CurrentValue.writeValue(Ramp01CurrentValue.value()+1);
      } 
      ledState= (ledState==HIGH) ? LOW : HIGH;       
      digitalWrite(LED_BUILTIN, ledState);

    }
  }
BLE.advertise();
Serial.println("Disconnected from central.");
Serial.print("Bluetooth device ");
Serial.print(BLE.address());
Serial.println(" advertising and waiting for connections...");
digitalWrite(LED_BUILTIN, LOW);

}

}


[[nodiscard]] void do_nothing(void)
{
    volatile int x = 42;
    (void)x; // Suppress unused variable warning

}