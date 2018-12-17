# Atomicus
WIFI + GUI

Atomic GUI 17DEC18.rar 
Is the latest compiled version of the Atomicus GUI for the ESP32 Hardware 

To run the program extract the rar and all of its contents to a local directory on your windows computer, the program has been tested to run standalone on windows 7 x64 and also windows 10 x64. 

The GUI is desiged to connect with the esp32 over UDP you have to know what the ip address is of the esp32 and of course you need to be on the same network. 

The network ssid and password can be set though the esp32's uart port or changing varibles in the C code base. 

This version fully supports UDP transmission 3 output on/off buttons and 3 pwm sliders spitting out values 0-255 

Known Issues:
We are currently working on the bug where we request a input vaule such as a button press or a sensor value and it needs to be sent back to the server address, so we are modifying the work flow of connect. when the connect button is pressed it sends the server ip (ip of device running the gui) to the esp32 so the esp32 knows where to send input udp packets 


To configure the Esp32 with the Arduino IDE 
Arduino IDE Version 1.8.5
ESP version Wroom32 

Depending on what ESP32 development board you are using, you will need the matching serial usb/uart drivers 

The tutorial below mentions the CP210x USB to UART Bridge VCP Drivers, but if you are using a FTDI based board you can skip that step. 
https://www.smart-prototyping.com/blog/How%20to%20Install-ESP32-Core-Arduino-IDE


