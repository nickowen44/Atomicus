/*
  ESP32 Atomicus
  Version 1.0a
  Date:07112018
  Nuvotion PTY LTD
*/


/*  References
   https://www.instructables.com/id/IOT-Made-Simple-Playing-With-the-ESP32-on-Arduino-/
*/


#include <WiFi.h>
#include <WiFiUdp.h>

// WiFi network name and password:
const char * networkName = "WiFi-D0B3";  // WIFI SSID
const char * networkPswd = "80927365";  //WIFI PASSWORD
//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char * udpAddress = "192.168.1.100";
const int udpPort = 3333;

//Are we currently connected?
boolean connected = false;

//The udp library class
WiFiUDP udp;

//Analog Inputs
// ESP32 ADCs have 12bits of resolution
// ADCs reading go tofrom 0 to 4,095
#define ANALOG_PIN_0 36  //light sensor
int analog_value = 0;

#define ANALOG_PIN_6 14  //temp sensor
int analog6_value = 0;


//Digital Outputs
int LED_GREEN = 4; // green led
int LED_RED = 5; // green led
int LED_YELLOW = 0; // green led


char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back

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

  //analog reads polling
  analog_value = analogRead(ANALOG_PIN_0);
 // Serial.println(analog_value);

  analog6_value = analogRead(ANALOG_PIN_6);
 // Serial.println(analog6_value);

/*
  digitalWrite(LED_GREEN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_RED, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_YELLOW, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(10);                       // wait for a second
  digitalWrite(LED_GREEN, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(LED_RED, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(LED_YELLOW, LOW);    // turn the LED off by making the voltage LOW
  delay(10);                       // wait for a second
*/


  //only send data when connected


  if (connected) {
    //Send a packet
    udp.beginPacket(udpAddress, udpPort);
    //udp.printf("Seconds since boot: %u", millis() / 1000);
    udp.printf("A0: %u", analog_value);
    udp.endPacket();
    
  }
  //Wait for 1 second
  delay(1000);


int packetSize = udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(udp.remotePort());

    // read the packet into packetBufffer
    int len = udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // send a reply, to the IP address and port that sent us the packet we received
    //udp.beginPacket(udp.remoteIP(), udp.remotePort());
    //udp.write(ReplyBuffer);
   // udp.endPacket();
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
