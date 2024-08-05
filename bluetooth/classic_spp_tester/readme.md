# Overview
I have not been able to find out how to communicate with Bluetooth serial using the module on the Arduino board that doesn't require flashing the firmware on the module which I am hesitant to do. But as a temporary solution I found this cheap bluetooth serial module https://www.amazon.com/dp/B07VL725T8?psc=1&ref=ppx_yo2ov_dt_b_product_details which I got to work at 921600 baud through the board's UART pins and a Windows COM port that magically appears after pairing:). Below are my instructions from the product review I wrote. 

# Instructions on using HC-05 version 2.0-20100601

I was able to get these modules to work in peripheral mode at 921600 baud. Below are some notes which apply to ones I purchased in December 2022 with a firmware version 2.0-20100601.

The modules have two modes: a configuration mode with AT commands and a serial communication mode for data transfer.

When the modules are powered up with the "EN" pin set to 3.3V or the button pressed they blink slowly (about once every 2 seconds) and can be configured with nonstandard AT commands. The speed is  always at 38400 baud. The device does not show up for pairing when in this mode.

When the modules are powered up with the "EN" pin set to 0V or is left unconnected they blink quickly (about twice a second) and are available for pairing with a default name of HC-05 and a default baud rate of 9600 baud for data communication.

To switch between the configuration mode and data communication mode requires cycling power on the device.

In the below configuration examples lines that start with # are comments. Also all commands are followed by a carriage return character (ASCII 13) followed by a newline character (ASCII 10).

```
# In configuration mode (slow blink) at 38400 baud I got the version with
AT+VERSION
# It returned
+VERSION:2.0-20100601
OK

# To get the UART speed I sent
AT+UART
# It returned (this was after I set it to 115200 from the default of 9600)
+UART:115200,0,0
OK

# To set the UART speed for 921600 (1 stop bit, No parity)
AT+UART=921600,0,0
# It returned
+UART:921600,0,0
OK

# To set the name of the device to "CH1" I sent
AT+NAME=CH1
# It returned
OK

# To get the name of the device I sent
AT+NAME
# It returned
NAME:CH1
OK
```

Once configured I cycled power with EN unconnected and it entered the data communication mode (fast blink). I was able to pair with it by finding the device "CH1" using Windows 10. Two COM ports appeared after pairing. The reason for two COM ports is related to RFCOMMs implementation and not this particular device. For me only one of the COM ports worked and its baud rate did not have to match the baud rate programmed into the HC-05 which I had set to 921600.

The PIN 1234 worked

