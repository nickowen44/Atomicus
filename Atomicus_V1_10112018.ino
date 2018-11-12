/*
  ESP32 Atomicus
  Version 1.0a
  Date:10112018
  Nuvotion PTY LTD
*/

/*  References
   https://www.instructables.com/id/IOT-Made-Simple-Playing-With-the-ESP32-on-Arduino-/
*/

#include <WiFi.h>
#include <WiFiUdp.h>

int splitPacket(void);
void processPacket(void);
void analogSend(void);

int splitSuccess = 0;

// WiFi network name and password:
const char * networkName = "OPTUS_BB9093";  // WIFI SSID
const char * networkPswd = "cedismagic32108";  //WIFI PASSWORD

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char * udpAddress = "192.168.0.5";
const int udpPort = 3333;

//Are we currently connected?
boolean connected = false;

//The udp library class
WiFiUDP udp;

//Analog Inputs
// ESP32 ADCs have 12bits of resolution
// ADCs reading go from 0 to 4,095
#define ANALOG_PIN_0 36  //light sensor
#define ANALOG_PIN_1 14
#define ANALOG_PIN_2 14
#define ANALOG_PIN_3 14
#define ANALOG_PIN_4 14
#define ANALOG_PIN_5 14
#define ANALOG_PIN_6 14  //temp 

int analog_value = 0;
int analog6_value = 0;

// MESSAGES
const char err_unrec[] = "Unrecognised the address/command: ";

// COMMANDS
const char LED_OFF[] = "LED_OFF";
const char LED_ON[] = "LED_ON";
const char ADC_READ[] = "ADC_READ";

// ADDRESSES
const char LED_RED_ADDRESS[] = "LED_RED";
const char LED_GREEN_ADDRESS[] = "LED_GREEN";
const char LED_YELLOW_ADDRESS[] = "LED_YELLOW";
const int numAnalogChannels = 6;
const char A[numAnalogChannels][4] = {"A0", "A1", "A2", "A3", "A4", "A5"};
int ADC_READ_FLAG = 0;




//Digital Outputs
int LED_GREEN = 4; // green led - LEDs are swapped on schematic?
int LED_RED = 5; // red led
int LED_YELLOW = 15; // yellow led - I cannot figure out why this does not work

char packetBuffer[255];         //buffer to hold incoming packet
char packetProcess[255];        // Using to process data
char packetAddress[255];        // Extracted address from packet
char packetCommand[255];        // Extracted command from packet
char packetDelimiter[2] = ":";  // Seperate incoming packet components by :
const int numTokens = 2;        // Number of components in an incoming packet
unsigned char newPacketReady = 1;       // System is ready to process a new packet
char  ReplyBuffer[] = "acknowledged";   // a string to send back

void setup() {
  
  // Initilize hardware serial:
  Serial.begin(115200);
  delay(100); // give me time to bring up serial monitor
  Serial.println("ESP32 Atomicus V1.0A");

  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);    // turn the LED off by making the voltage LOW

  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);    // turn the LED off by making the voltage LOW

  pinMode(LED_YELLOW, OUTPUT);
  digitalWrite(LED_YELLOW, LOW);    // turn the LED off by making the voltage LOW


  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
}

void loop() {
  
  // Is delay necessary?
  delay(1000);

  // Receive a packet
  int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(udp.remotePort());

    // Read the packet into packetBufffer
    int len = udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = '\0';
    }
    Serial.println("Contents:");
    Serial.println(packetBuffer);
  
  // Copy packet to begin processing
  if (newPacketReady) {
    newPacketReady = 0;
    strcpy(packetProcess, packetBuffer);
  
    if (splitPacket()) {
      processPacket();
      if (ADC_READ_FLAG) {
        analogSend();
      }
    } else {
      newPacketReady = 1;
    }
  
  }

   // Send a reply, to the IP address and port that sent us the packet we received
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    int i;
    for(i = 0; i < strlen(ReplyBuffer); i++){
      udp.write(ReplyBuffer[i]);
    }
    udp.endPacket();
  }
  
}

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

//wifi event handler
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
  char* pointers[numTokens] = {packetAddress, packetCommand};
  int i = 0;
  
  // Split the packet into components
  token = strtok(packetProcess, packetDelimiter );
  while(token != NULL){
    strcpy( pointers[i], token );
    token = strtok(NULL, packetDelimiter );
    i++;
  }
  // Check to see if corect number of splits in packet
  if (i != numTokens ) {
    Serial.println("Packet Splitting Failed");
    return 0;
  }
  return 1;

}

void processPacket(void){

  
  // LED COMMANDS
  if (strcmp(packetAddress, LED_GREEN_ADDRESS) == 0) {
    if (strcmp(packetCommand, LED_ON) == 0) {
      digitalWrite(LED_GREEN, HIGH);
    } else if (strcmp(packetCommand, LED_OFF) == 0){
      digitalWrite(LED_GREEN, LOW);
    }
    
  } else if (strcmp(packetAddress, LED_RED_ADDRESS) == 0){
    
    if (strcmp(packetCommand, LED_ON) == 0) {
      digitalWrite(LED_RED, HIGH); 
    } else if (strcmp(packetCommand, LED_OFF) == 0) {
      digitalWrite(LED_RED, LOW);
    }
    
  } else if (strcmp(packetAddress, LED_YELLOW_ADDRESS) == 0){
    
    if (strcmp(packetCommand, LED_ON)) {
      digitalWrite(LED_YELLOW, HIGH); 
    } else if (strcmp(packetCommand, LED_OFF) == 0) {
      digitalWrite(LED_YELLOW, LOW);
    }
    
  } else if (strncmp(packetAddress, A[0], 2) == 0) { 
  
    if (strncmp(packetCommand, ADC_READ, 8) == 0 ) {
      ADC_READ_FLAG = 1;
    }

  } else if (strncmp(packetAddress, A[1], 2) == 0) { 
    
    if (strncmp(packetCommand, ADC_READ, 8) == 0 ) {
      ADC_READ_FLAG = 1;
    }

  } else if (strncmp(packetAddress, A[2], 2) == 0) { 
  
    if (strncmp(packetCommand, ADC_READ, 8) == 0 ) {
      ADC_READ_FLAG = 1;
    }

  } else if (strncmp(packetAddress, A[3], 2) == 0) { 
  
    if (strncmp(packetCommand, ADC_READ, 8) == 0 ) {
      ADC_READ_FLAG = 1;
    }

  } else if (strncmp(packetAddress, A[4], 2) == 0) { 
  
    if (strncmp(packetCommand, ADC_READ, 8) == 0 ) {
      ADC_READ_FLAG = 1;
    }

  } else if (strncmp(packetAddress, A[5], 2) == 0) { 
  
    if (strncmp(packetCommand, ADC_READ, 8) == 0 ) {
      ADC_READ_FLAG = 1;
    }

  } else {
    Serial.print(err_unrec);
    Serial.println(packetAddress);
  }
  
  newPacketReady = 1;
}

// Only use after packets have been received
void analogSend(void) {
  
  int adcPinNum = packetAddress[1] - 48; // Convert ASCII to int
  
  switch(adcPinNum){ 
    case(0) :
      analog_value = analogRead(ANALOG_PIN_0);
      break;
    case(1) :
      analog_value = analogRead(ANALOG_PIN_1);
      break;
    case(2) :
      analog_value = analogRead(ANALOG_PIN_2);
      break;
    case(3) :
      analog_value = analogRead(ANALOG_PIN_3);
      break;
    case(4) :
      analog_value = analogRead(ANALOG_PIN_4);
      break;
    case(5) :
      analog_value = analogRead(ANALOG_PIN_5);
      break;
  } 
  
  if (connected) {
    //Send a packet
    udp.beginPacket(udpAddress, udpPort);
    //udp.printf("Seconds since boot: %u", millis() / 1000);
    //udp.printf("%s: %u", A[adcPinNum], analog_value);
    udp.printf("%s: %u",A[adcPinNum],  analog_value);
    udp.endPacket();
    
  }
  ADC_READ_FLAG = 0;
}
