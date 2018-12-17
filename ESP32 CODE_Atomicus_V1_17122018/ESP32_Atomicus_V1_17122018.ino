/*
  ESP32 Atomicus
  Version 1.0b 
  Date:03DEC18
/*  References
   https://www.instructables.com/id/IOT-Made-Simple-Playing-With-the-ESP32-on-Arduino-/
*/

#include <WiFi.h>
#include <WiFiUdp.h>

// -------------------------- WIFI CONSTANTS ---------------------------------//
// WiFi network name and password:
char networkName[64] = "NuvotionHQ";  // WIFI SSID
char networkPswd[64] = "Portsofparadice45";  //WIFI PASSWORD

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
char udpAddress[64] = "10.1.1.186";
int udpPort = 3333;
int udpPortOld = udpPort;

boolean connected = false; //Are we currently connected?

//The udp library class
WiFiUDP udp;

//--------------------- ANALOG CONSTANTS/VARIABLES ---------------------------//
// ESP32 ADCs have 12bits of resolution
// ADCs reading go from 0 to 4,095

#define ANALOG_PIN_0 36  //light sensor
#define ANALOG_PIN_3 39
#define ANALOG_PIN_4 32
#define ANALOG_PIN_5 33
#define ANALOG_PIN_6 34  //temp 
#define ANALOG_PIN_7 35

const int numAnalogChannels = 6; // This will need changing in future, need to know what pins are what
const int numAnalogChannelsAll = 8;
const char A[numAnalogChannelsAll][4] = {"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7"};
int ADC_READ_FLAG = 0;

const int analogPins[numAnalogChannelsAll] = {
  ANALOG_PIN_0, 0, 0, ANALOG_PIN_3, ANALOG_PIN_4,
  ANALOG_PIN_5, ANALOG_PIN_6, ANALOG_PIN_7
};

// Values read from ADCs
int analogPinValues[numAnalogChannelsAll] = {0, 0, 0, 0, 0, 0, 0, 0};
// Enable whether or not the channels are continuously read
boolean analogContEnable[numAnalogChannelsAll] = {
  false, false, false, false, false, false, false, false
};


//------------------- DIGITAL INPUT CONSTANTS/VARIABLES ----------------------//

const int numDigitalInputChannels = 5;
const int digitalInputChannel[numDigitalInputChannels] = {0, 1, 2, 3, 35};
int digital_value = 0;
int input_flag = 0;


//------------------ DIGITAL OUTPUT CONSTANTS/VARIABLES ----------------------//
const int LED_GREEN   = 5;   // green led
const int LED_RED     = 4;   // red led
const int LED_YELLOW  = 15;  // yellow led
const int numDigitalOutputChannels = 3;
const int digitalOutputChannels[numDigitalOutputChannels] = {
  LED_GREEN, LED_RED, LED_YELLOW
};
int output_flag = 0;

//--------------------- PWM OUTPUT CONSTANTS/VARIABLES -----------------------//

// RGB LED labelled pins to ESP GPIO
#define D6 18 // Green
#define D7 19 // Blue
#define D8 21 // Red

const int freq = 12000;
const int resolution = 8;

const int numPWMChannels = 3;
const int PWMChannels[numPWMChannels] = {0, 1, 2};
const int PWMPins[numPWMChannels] = {D6, D7, D8};
int pwm_flag = 0;

// STRING CONSTANTS
const char null_byte = '\0';

// MESSAGES
const char err_unrec[] = "Unrecognised the address/command: ";
const char udp_request[] = "Please enter a new udp port";
const char ip_request[] = "Please enter a new IP address";
const char request_ssid[] = "Please enter a new ssid";
const char request_password[] = "Please enter a new password";
const char disp_udp[] = "The udp port is: ";
const char disp_ip[] = "The IP address is: ";
const char disp_dest_ip[] = "The destination IP address is: ";
const char disp_board_ip[] = "The IP address of the board is: ";
const char contents[] = "Contents";
const char connect_string[] = "The Board is now connected to: ";


const char receivedMessage[] = "Received packet of size ";
const char receivedFromMesage[] = "From ";
const char receivedPortMessage[] = ", port ";

// COMMANDS - UDP
const char digital_write    = 'd';
const char digital_read     = 'r';
const char analog_read      = 'a';
const char pwm_write        = 'p';
const char cont_analog_read = 'c';
const char ip_config        = 'i';
const char udp_port_config  = 'u';
const char connection_set   = 's';

// COMMANDS - Serial

// The commands change_udp and change_ip are changing the 
// destination of packets to be sent to. NOTE: If a computer 
// requests a one time read from the ESP, the ESP will send back to the computer
// Only the continuous sends go back to this IP address
const char change_udp[] = "udpc\r\n";
const char change_ip[] = "ipc\r\n";

// Request the IP address of the server
const char request_ip[] = "ip\r\n";

// Commands to change / get network details
const char request_net[] = "net\r\n";
const char change_network[] = "netc\r\n";

// Request IP address of board
const char request_ip_board[] = "ip_board\r\n";


//--------------------------- PACKET VARIABLES -------------------------------//
char packetBuffer[255];         // Buffer to hold incoming packet
char packetProcess[255];        // Using to process data
char packetAddress[255];        // Extracted address from packet
char packetCommand1[255];       // Extracted commands from packet
char packetCommand2[255];
int packetAddressNumber;        // Numbers from the packet
int packetCommand2Number;
int splitSuccess = 0;
const int numTokens = 3;        // Number of components in an incoming packet
char packetDelimiter[2] = ":";  // Seperate incoming packet components by :

// System is ready to process a new packet
//This currently isn't necessary as the system isn't interrupt driven
unsigned char newPacketReady = 1;

char  ReplyBuffer[] = "ok";   // a string to send back

//---------------------------- SERIAL VARIABLES ------------------------------//
String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete
bool ip_change_flag = false;
bool udp_change_flag = false;
bool net_change_flag = false;
bool received_ssid_serial = false;

//----------------------------------------------------------------------------//
//---------------------------- PROGRAM ---------------------------------------//
//----------------------------------------------------------------------------//
void setup() {
  int i;
  // Initilize hardware serial:
  Serial.begin(115200);
  delay(100); // give me time to bring up serial monitor
  Serial.println("ESP32 Atomicus V1.0A");

  // Setup Outputs
  for (i = 0; i < numDigitalOutputChannels; i++) {
    pinMode(digitalOutputChannels[i], OUTPUT);
    digitalWrite(digitalOutputChannels[i], LOW);
  }

  // Setup Inputs - Default to pullup
  for (i = 0; i < numDigitalInputChannels; i++) {
    pinMode(digitalInputChannel[i], INPUT_PULLUP);
  }

  // Setup PWM channels
  for (i = 0; i < numPWMChannels; i++) {
    ledcSetup(PWMChannels[i], freq, resolution);
    ledcAttachPin(PWMPins[i], PWMChannels[i]);
  }

  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
}

void loop() {

  int i; // Index variable
  // Is delay necessary?
  delay(1);

  // Receive a packet
  int packetSize = udp.parsePacket();
  // Serial.println(packetSize);

  if (packetSize) {
    Serial.print(receivedMessage);
    Serial.println(packetSize);
    Serial.print(receivedFromMesage);
    IPAddress remoteIp = udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(receivedPortMessage);
    Serial.println(udp.remotePort());

    // Read the packet into packetBufffer
    int len = udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = null_byte;
    }
    Serial.println(contents);
    Serial.println(packetBuffer);

    // Copy packet to begin processing
    if (newPacketReady) {
      newPacketReady = 0;
      strcpy(packetProcess, packetBuffer);

      if (splitPacket()) {
        executePacket();
        if (ADC_READ_FLAG) {
          analogSend(packetAddressNumber);
        }
        if (input_flag) {

          digitalSend();
        }
      } else {
        newPacketReady = 1;
      }

    }
  /* DID THIS PART DIFFERENTLY
    if (connect_request_flag) {

      udp.beginPacket(udpAddress, udpPort);
      for (i = 0; i < strlen(ReplyBuffer); i++) {
        udp.write(ReplyBuffer[i]);
      }
      udp.endPacket();
      connect_request_flag = false;
      
    }
    */
/*
    // Send a reply, to the IP address and port that sent us the packet we received
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
   
  for (i = 0; i < strlen(ReplyBuffer); i++) {
      udp.write(ReplyBuffer[i]);
    }
    
    
    udp.endPacket();
*/

    
  }

  // Continuously send analog values to server if enabled
  for (i = 0; i < numAnalogChannelsAll; i++) {
    if (analogContEnable[i]) {
      analogPinValues[i] = analogRead(analogPins[i]);
      analogSendToServer(i);
      Serial.println(analogPinValues[i]);
    }
  }

  // Process any serial inputs
  serialProcessing();
  //  Serial.println("looping");

}

// Process inputs from Serial
void serialProcessing(void) {
  //b Serial.println("checking serial");
  while (Serial.available()) {
    inputString = Serial.readString();
    char inString[255];
    inputString.toCharArray(inString, 255);


    if (strcmp(request_ip, inString) == 0) {
      Serial.print(disp_ip);
      Serial.println(udpAddress);
      Serial.print(disp_udp);
      Serial.println(udpPort);
    }
    if (strcmp(request_net, inString) == 0) {
      Serial.println(networkName);
      Serial.println(networkPswd);
    }
    if (strcmp(request_ip_board, inString) == 0) {
      Serial.print(disp_board_ip);
      Serial.println(WiFi.localIP());
    }
    if (ip_change_flag) {
      strcpy(udpAddress, inString);
      ip_change_flag = false;
      Serial.print(disp_ip);
      Serial.println(udpAddress);
    }

    if (udp_change_flag) {
      udpPortOld = udpPort;
      udpPort = atoi(inString);
      udp_change_flag = false;
      Serial.print(disp_udp);
      Serial.println(udpPort);
      //udp.end(WiFi.localIP(), udpPortOld);
      udp.stop();
      udp.begin(WiFi.localIP(), udpPort);

    }

    if (net_change_flag) {
      if (!received_ssid_serial) {
        strcpy(networkName, inString);
        received_ssid_serial = true;
        Serial.println(request_password);
      } else {
        strcpy(networkPswd, inString);
        net_change_flag = false;
        received_ssid_serial = false;
        Serial.println(networkName);
        Serial.println(networkPswd);
        connectToWiFi(networkName, networkPswd);
      }


    }

    if (strcmp(change_ip, inString) == 0) {
      Serial.println(ip_request);
      ip_change_flag = true;
    }

    if (strcmp(change_udp, inString) == 0) {
      Serial.println(udp_request);
      udp_change_flag = true;
    }

    if (strcmp(change_network, inString) == 0) {
      Serial.println(request_ssid);
      net_change_flag = true;
    }

  }
}

// Attempt to connect to the network
void connectToWiFi(const char * ssid, const char * pwd) {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);

  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

// Wifi event handler
void WiFiEvent(WiFiEvent_t event) {
  
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      //When connected set
      Serial.print("WiFi connected! IP address: ");
      Serial.println(WiFi.localIP());
    
      //initializes the UDP state
      //This initializes the transfer buffer
      udp.begin(WiFi.localIP(), udpPort);
      connected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      connected = false;
      break;
  }
}

// Splits the packet packetProcess into Address/Command
int splitPacket(void) {

  char* token;
  char* pointers[numTokens] = {packetCommand1, packetAddress, packetCommand2};
  int i = 0;
  // Split the packet into components
  token = strtok(packetProcess, packetDelimiter );
  while (token != NULL) {
    strcpy( pointers[i], token );
    token = strtok(NULL, packetDelimiter );
    i++;
  }

  if (i == 2 || i == 3) {
    if (strlen(packetCommand1) != 1) {
      Serial.println("Incorrect command length");
    }
    packetAddressNumber = (int) strtol(packetAddress, NULL, 10);

    if (i == 3) {
      packetCommand2Number = (int) strtol(packetCommand2, NULL, 10);
    }
  }  else {
    Serial.println("Packet Splitting Failed");
    return 0;
  }

  return 1;

}

void executePacket(void) {
  int i;

  switch (packetCommand1[0]) {
    case digital_write : {
        for (i = 0; i < numDigitalOutputChannels; i++) {
          if (packetAddressNumber == digitalOutputChannels[i]) {
            output_flag = 1;
          }
        }
        if (output_flag) {
          switch (packetCommand2[0]) {
            case 'h':
              digitalWrite(packetAddressNumber, HIGH);
              Serial.print("D");
              Serial.print(packetAddressNumber);
              Serial.println(" HIGH");
              break;
            case 'l':
              digitalWrite(packetAddressNumber, LOW);
              Serial.print("D");
              Serial.print(packetAddressNumber);
              Serial.println(" LOW");
              break;
            default:
              Serial.print(err_unrec);
              Serial.println(packetCommand2[0]);
          }
          output_flag = 0;
        } else {
          Serial.print("D");
          Serial.print(packetAddressNumber);
          Serial.println(" is not a digital output.");
        }

        break;
      }
    case analog_read : {

        analogPinValues[packetAddressNumber] = analogRead(analogPins[packetAddressNumber]);
        Serial.print(A[packetAddressNumber]);
        Serial.print(" value: ");
        Serial.println(analogPinValues[packetAddressNumber]);
        ADC_READ_FLAG = 1;
        break;
      }
    case digital_read : {

      input_flag = 0;
      // Check to see if address corresponds to an input pin
      for (i = 0; i < numDigitalInputChannels; i++) {
        if (packetAddressNumber == digitalInputChannel[i]) {
          input_flag = 1;
        }
      }
      if (input_flag) {
        digital_value = digitalRead(packetAddressNumber);
      } else {
        Serial.print("D");
        Serial.print(packetAddressNumber);
        Serial.println(": is not a digital input.");
      }
      break;
    }
    case pwm_write : {
      for (i = 0; i < numPWMChannels; i++) {
        if (packetAddressNumber == PWMChannels[i]) {
          pwm_flag = 1;
        }
      }
      if (pwm_flag) {
        ledcWrite(packetAddressNumber, packetCommand2Number);
      } else {
        Serial.print("P");
        Serial.print(packetAddressNumber);
        Serial.println(": is not a pwm channel.");
      }
      break;
    }
    case  cont_analog_read : {
      if (packetAddressNumber >= 0 && packetAddressNumber < 8) {
        analogContEnable[packetAddressNumber] = (bool)packetCommand2Number;
      }
      break;
    }
    case ip_config : { 
      switch (packetAddressNumber) {
        case 0 : {
          Serial.print(disp_dest_ip);
          Serial.println(udpAddress);
          udp_send_address();
          break;
        }
        case 1 : {
          strcpy(udpAddress, packetCommand2);
          Serial.print(disp_dest_ip);
          Serial.println(udpAddress);
          udp_send_address();
          break;
        }
        default : {
          Serial.print(err_unrec);
          Serial.println(packetAddress);
        }
      }
      break;
    }
    case udp_port_config : {
      switch (packetAddressNumber) {
        case 0 : {
          udp_send_port();
          break;
        }
        case 1 : {
          udpPortOld = udpPort;
          udpPort = atoi(packetCommand2);
          Serial.print(disp_udp);
          Serial.println(udpPort);
          udp.stop();
          udp.begin(WiFi.localIP(), udpPort);
          udp_send_port();
          break;
        }
        default : {
          Serial.print(err_unrec);
          Serial.println(packetAddress);
        }
      }
      break;
    }
    case connection_set : {
      strcpy(udpAddress, packetAddress);
      udpPort = packetCommand2Number;
      server_send_acknowledge();
      break;
    }
    default :  {
        Serial.print(err_unrec);
        Serial.println(packetCommand1[1]);
        udp.stop();
        udp.begin(WiFi.localIP(), udpPort);
      }
  }

  newPacketReady = 1;
}

// Send an ADC read to the requester
void analogSend(int address) {

  if (connected) {
    //Send a packet
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    //udp.printf("Seconds since boot: %u", millis() / 1000);
    //udp.printf("%s: %u", A[adcPinNum], analog_value);
    udp.printf("%s: %u", A[address],  analogPinValues[address]);
    udp.endPacket();

  }
  ADC_READ_FLAG = 0;
}

// Send ADC reads to the server
void analogSendToServer(int address) {

  if (connected) {
    //Send a packet
    udp.beginPacket(udpAddress, udpPort);
    //udp.printf("Seconds since boot: %u", millis() / 1000);
    //udp.printf("%s: %u", A[adcPinNum], analog_value);
    udp.printf("%s: %u", A[address],  analogPinValues[address]);
    udp.endPacket();

  }
  ADC_READ_FLAG = 0;
}

// Send the digital read to the requester
void digitalSend(void) {
  if (connected) {
    //Send a packet
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("D%d: %u", packetAddressNumber,  digital_value);
    udp.endPacket();
  }
  input_flag = 0;
}

void udp_send_address(){

  if (connected) {
    //Send a packet
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("%s: %s", disp_dest_ip,  udpAddress);
    udp.endPacket();
  }

}
void udp_send_port(){

  if (connected) {
    //Send a packet
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.printf("%s: %u", disp_udp,  udpPort);
    udp.endPacket();

  }

}

void server_send_acknowledge() {
  if (connected) {
    //Send a packet
    udp.beginPacket(udpAddress, udpPort);
   // udp.printf("Connected to IP: %s and UDP port : %u",udpAddress, udpPort );
    udp.printf("Connected");
    udp.endPacket();
    Serial.print("Connected to IP: ");
    Serial.println(udpAddress);
    Serial.print("UDP: ");
    Serial.println(udpPort);
  }
}

