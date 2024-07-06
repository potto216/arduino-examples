/*
  The purpose of this program is to demonstrate how the Arduino MKR WiFi 1010 can use the HC-05 Bluetooth Classic (BR/EDR) hardware module which supports SPP Bluetooth serial port protocol (RFCOMM)
  to  transmit serial data over Bluetooth.

  The particular hardware module used is the HC-05 version 2.0-20100601
  
  This program allows the user to run the HC-05 in two operating modes:
  1. Configuration mode: Configure the HC-05 with AT commands 
  2. Serial data mode: Use the HC-05 to transfer serial data based on the configuration settings.

  To select the HC-05 operating mode the user needs to start the MKR WiFi 1010 board with pin D0 acting as a logic input depending on if it is connected to a high or low output will choose the operating mode. For convenience pin D1 will output a logic low and pin D2 will output a logic high and Pin D0 can be connected to either to select the mode.  
  The modes are selected by
  1. Configuration mode: Connect pin D0 to a logic high such as pin D2 
  2. Serial data mode: Connect pin D0 to a logic low such as pin D2 

  The selected mode will then choose the logic level on pin D3 which is a logic output and will be connected to the HC-05 EN input pin with the following modes
  1. Configuration mode: pin D3 outputs a logic high to HC-05 EN
  2. Serial data mode: pin D3 outputs a logic low to HC-05 EN

  The hardware setup is to connect the HC-05 to the MKR WiFi 1010 board with the following pinout is:
  MKR-WiFi-1010       HC-05 
  5V           ->     5V   
  GND          ->     GND
  TX (Pin 14)  ->     RXD (input)
  RX (Pin 13)  <-     TXD (output)
  D3           ->     EN (input)

The MKR WiFi 1010 USB serial port which uses the Serial object will be configured for 115200 and allow the user to type AT commands in configuration mode and receive output from the HC-05 and receive log messages about the program operation when in serial data mode.

The HC-05 uses the Serial1 object to communicate with the MKR WiFi 1010 board.

Some details about the HC-05 board are:

1. This is for the HC-05 with a firmware version 2.0-20100601. Other firmware versions may operate differently

2. When the modules are powered up with the "EN" pin set to 3.3V or the button pressed they blink slowly (about once every 2 seconds) and can be configured with nonstandard AT commands. The speed is  always at 38400 baud. The device does not show up for pairing when in this mode.

3. When the modules are powered up with the "EN" pin set to 0V or is left unconnected they blink quickly (about twice a second) and are available for pairing with a default name of HC-05 and a default baud rate of 9600 baud for data communication.

4. To switch between the configuration mode and data communication mode requires cycling power on the device.

5. In configuration mode all commands are followed by a carriage return character (ASCII 13) followed by a newline character (ASCII 10).


In Serial data mode the program monitors the serial line for the following text based commands and uses a switch statement to call the functions with the name. Valid names are:

setControlPinLow
setControlPinHigh
help

Use constants whenever possible for values like data rates etc. Add log statements about the program operation over the Serial object. The code below is a rough example which does not do everything correctly and needs to be completed and organized.

*/

#define CONFIG_MODE_BAUD 38400  // Baud rate for HC-05 configuration mode
#define DATA_MODE_BAUD 921600   // Baud rate for HC-05 data communication mode (the default is 9600)

// Pin definitions
#define MODE_SELECT_PIN 0             // Selecting the operating mode
#define DATA_MODE_INDICATOR_PIN 1     // as an output for logic low
#define CONFIG_MODE_INDICATOR_PIN 2   // as an output for logic high
#define HC_05_EN_PIN 3                // Connect to HC-05 EN pin
#define CONTROL_PIN 4                 // Used to demonstrate control over the serial port

#define CONFIG_MODE_VALUE HIGH
#define DATA_MODE_VALUE LOW

int mode = DATA_MODE_VALUE; // data is the default mode

HardwareSerial& USBSerial = Serial;
HardwareSerial& BluetoothSerial = Serial1;

void setup() {
  // Setup the HC-05
  pinMode(CONFIG_MODE_INDICATOR_PIN, OUTPUT);
  digitalWrite(CONFIG_MODE_INDICATOR_PIN, CONFIG_MODE_VALUE);

  pinMode(DATA_MODE_INDICATOR_PIN, OUTPUT);  
  digitalWrite(DATA_MODE_INDICATOR_PIN, DATA_MODE_VALUE); 
  
  pinMode(MODE_SELECT_PIN, INPUT);
  mode = digitalRead(MODE_SELECT_PIN); 

  pinMode(HC_05_EN_PIN, OUTPUT);
  digitalWrite(HC_05_EN_PIN, mode == CONFIG_MODE_VALUE ? CONFIG_MODE_VALUE : DATA_MODE_VALUE);

  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, LOW);

  // Start USB serial communication
  USBSerial.begin(115200);
  
  // Comment this out if you will not be using the serial port otherwise the program will stall waiting for it.
  while (!USBSerial); // Wait for serial port to connect
 
  // Initialize BluetoothSerial based on selected mode
  if (mode == CONFIG_MODE_VALUE) {
    USBSerial.println("Verify the HC-05 module is blinking slow. If not you may need to manually cycle power on the HC-05 module to get it in configuration mode.");
    USBSerial.println("Configuration Mode: Enter AT Commands.");    
    BluetoothSerial.begin(CONFIG_MODE_BAUD); 
    USBSerial.print("Baud rate for configuration: ");
    USBSerial.println(CONFIG_MODE_BAUD);
  } else {
    USBSerial.println("Verify the HC-05 module is blinking fast and you have previously set the baud rate with the AT commands.");
    USBSerial.println("Serial Data Mode: Ready for data communication.");
    BluetoothSerial.begin(DATA_MODE_BAUD);
    USBSerial.print("Baud rate for data communication: ");
    USBSerial.println(DATA_MODE_BAUD);

    USBSerial.println("Enter commands (setControlPinLow, setControlPinHigh, help):");
  }
}

void loop() {
  
  // Handle incoming commands from Bluetooth Serial
  static String inputCommand = "";

if (mode == CONFIG_MODE_VALUE) {
  if (USBSerial.available()) 
  { 
    
    char ch = USBSerial.read();
    // This is specific to the HC-05 that it needs just a newline translated to a carrage return and then newline.
    //TODO: you may need to drop any carrage returns that are sent
    if(ch == 10)
    {
    BluetoothSerial.write(13);  
    BluetoothSerial.write(10);
    }
    else
    {
      BluetoothSerial.write(ch);
    }
  }

  // Transfer data from HC-05 to the USB serial
  if (BluetoothSerial.available()) {
    USBSerial.write(BluetoothSerial.read());
  }
}
else   // if in Bluetooth serial data mode
{
  // Transfer data from USB to HC-05
  if (USBSerial.available()) {
    BluetoothSerial.write(USBSerial.read());
  }

  if (BluetoothSerial.available()) {
    char ch = BluetoothSerial.read();
    BluetoothSerial.write(ch); // echo character back
    if (ch == '\n' || ch == '\r') {
      if (inputCommand.length() > 0) {
        handleCommand(inputCommand);
        inputCommand = ""; // Reset command string after handling
      }
    }  else if (!isspace(ch)) { // Skip whitespace
      inputCommand += ch; // Accumulate characters into command
    }
  }

}


}

void handleCommand(String command) {
  if (command == "setControlPinLow") {
    setControlPinLow();
  } else if (command == "setControlPinHigh") {
    setControlPinHigh();
  } else if (command == "help") {
    help();
  } else {
    USBSerial.print("Invalid command: ");
    USBSerial.println(command);

    BluetoothSerial.print("Invalid command: ");
    BluetoothSerial.println(command);
  }
}

void setControlPinLow() {
  digitalWrite(CONTROL_PIN, LOW);
  USBSerial.print("Set Control Pin (");
  USBSerial.print(CONTROL_PIN);
  USBSerial.println(") Low.");

  BluetoothSerial.print("Set Control Pin (");
  BluetoothSerial.print(CONTROL_PIN);
  BluetoothSerial.println(") Low.");
}

void setControlPinHigh() {
  digitalWrite(CONTROL_PIN, HIGH);
  USBSerial.print("Set Control Pin (");
  USBSerial.print(CONTROL_PIN);
  USBSerial.println(") High.");

  BluetoothSerial.print("Set Control Pin (");
  BluetoothSerial.print(CONTROL_PIN);
  BluetoothSerial.println(") High.");
}

void help() {
  USBSerial.println("Valid commands: setControlPinLow, setControlPinHigh, help");
  BluetoothSerial.println("Valid commands: setControlPinLow, setControlPinHigh, help");
}
