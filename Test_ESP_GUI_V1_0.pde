/**
ESP-WROOM-32 GUI
Written by: David Lynch
Version: 1.0
Date: 15/11/2018
Communicates with a ESP board over UDP
*/

// --------------------------- UDP Variables/Constants -----------------------------/
import hypermedia.net.*;            // Import the UDP library
UDP udp;                            // Define the UDP object
String ipAddress = "192.168.0.10";  // IP Address of the microcontroller
int udpPort = 3333;                 // UDP port to send data through


// ------------------------- ControlP5 Variables/Constants -------------------------/
import controlP5.*;                 // Import the ControlP5 Library
ControlP5 cp5;                      // Define the ControlP5 object
int r,g,b;                          // RGB values of colourwheel object

int[] RED     = {255, 0  , 0  };
int[] ORANGE  = {255, 165, 0  };
int[] YELLOW  = {255, 255, 0  };
int[] GREEN   = {0  , 255, 0  };
int[] BLUE    = {0  , 0  , 255};
int[] PURPLE  = {160, 32 , 240};

String[][] colourWheelButtonStrings = {{"R Val On", "G Val On", "B Val On", "All On", "R", "B"}, 
                                      {"R Val Off", "G Val Off", "B Val Off", "All Off", "G", "Y"}};
String[][] colourWheelButtonLabels = {{"RGB_RED_ON", "RGB_GREEN_ON", "RGB_BLUE_ON", "RGB_ALL_ON", "RGB_RED", "RGB_BLUE"}, 
                                      {"RGB_RED_OFF", "RGB_GREEN_OFF", "RGB_BLUE_OFF", "RGB_ALL_OFF", "RGB_GREEN", "RGB_YELLOW"}};
String[][] ledButtonStrings = {{"Y LED on", "Y LED off"},{"R LED on", "R LED off"},{"G LED on", "G LED off"}};
String[][] ledButtonLabels = {{"Y_LED_ON", "Y_LED_OFF"},{"R_LED_ON", "R_LED_OFF"},{"G_LED_ON", "G_LED_OFF"}};  

int numDigitalInputs = 4;
String[] digitalInputStrings = {"Read D0", "Read D1", "Read D2", "Read D3"};
String[] digitalInputLabels = {"READ_D0", "READ_D1", "READ_D2", "READ_D3"};
String nr = "Not Read";
String[] digitalReads = {nr + " 0", nr+ " 1", nr+ " 2", nr+ " 3"};
boolean[] toDigitalRead = {false, false, false, false};

String not_on = "Not on";
String[] analogReads = {not_on, not_on, not_on, not_on, not_on, not_on};
boolean[] analogChannelOn = {false, false, false, false, false, false};
int numAnalogInputs = 6;

int pixelOffset = 23;
/**
 * init
 */
void setup() {
  
  // Create a new font compatible with ControlP5 - Needed to adjust size
  PFont pfont = createFont("Arial",20,true); 
  ControlFont font = new ControlFont(pfont,32);
  int pixelOffset = 23;   // Font height is 46 pixels at 32pt
 
  // Draw the Canvas
  size(1000,1080);
  //PImage img = loadImage("background.png"); // Placeholder Image, change in future
  background(0);
  
  strokeWeight(5);
  textAlign(CENTER);
  
  
  cp5 = new ControlP5( this );
  cp5.addColorWheel("c" , 0 , height/2 ,  min(width/4, height/2) ).setRGB(color(128,0,255)); // Add a colour wheel in for the RGB LED on the board
  for (int i = 0; i <= 2; i++) {
    cp5.addButton(ledButtonLabels[i][0]).setPosition(i*(width/2)/3, 0       ).setSize((width/2)/3, height/4  ).setFont(font).setLabel(ledButtonLabels[i][0]);
    cp5.addButton(ledButtonLabels[i][1]).setPosition(i*(width/2)/3, height/4).setSize((width/2)/3, height/4  ).setFont(font).setLabel(ledButtonLabels[i][1]);
  }
  for (int i = 0; i <= 5; i++) {
    cp5.addButton(colourWheelButtonLabels[0][i]).setPosition(width/4 , height/2 + i*(height/2)/6 ).setSize((width/2)/4, (height/2)/6  ).setFont(font).setLabel(colourWheelButtonStrings[0][i]);
    cp5.addButton(colourWheelButtonLabels[1][i]).setPosition(3*width/8, height/2 + i*(height/2)/6 ).setSize((width/2)/4, (height/2)/6  ).setFont(font).setLabel(colourWheelButtonStrings[1][i]);
  }
  
  for (int i = 0; i <= 3; i++){
    cp5.addButton(digitalInputLabels[i]).setPosition(width/2 + i*(width/2)/4 ,0).setSize((width/2)/4, (height/4)  ).setFont(font).setLabel(digitalInputStrings[i]);
  }
  textSize(32);
  text("Pins"    , width/2,                 height/2  + pixelOffset, (width/2)/4 , (height/2)/5 );
  text("Values"  , width/2 + 1*(width/2)/4, height/2  + pixelOffset, (width/2)/4 , (height/2)/5 );
  text("On"      , width/2 + 2*(width/2)/4, height/2  + pixelOffset, (width/2)/4 , (height/2)/5 );
  text("Off"     , width/2 + 3*(width/2)/4, height/2  + pixelOffset, (width/2)/4 , (height/2)/5 );
  
  for (int i = 0; i <= 7; i++) {
   line(width/2, height/2 + i*(height/2)/7, width, height/2 + i*(height/2)/7);
   
  
  }
  for (int i = 0; i <= 6; i++) {
    text("A" + String.valueOf(i) , width/2, height/2 + (i+1)*(height/2)/7 + pixelOffset, (width/2)/4, (height/2)/7 );
  }
  for (int i = 0; i <= 5; i++) {
    cp5.addButton("A" + String.valueOf(i) + "On" ).setPosition(width/2 + 2*(width/2)/4, height/2 + (i+1)*(height/2)/7 ).setSize((width/2)/4, (height/2)/7  ).setFont(font).setLabel("A" + String.valueOf(i) + " on");
    cp5.addButton("A" + String.valueOf(i) + "Off" ).setPosition(width/2 + 3*(width/2)/4, height/2 + (i+1)*(height/2)/7 ).setSize((width/2)/4, (height/2)/7  ).setFont(font).setLabel("A" + String.valueOf(i) + " off");
  }
  for (int i = 0; i <= 4; i++) {
   line(width/2 + i*(width/2)/4, height/2, width/2 + i*(width/2)/4, height); 
  }
  
  udp = new UDP( this, 3333 ); // Initilise the UDP object
  udp.log( true );             // Printout the connection activity -- DEBUG ONLY
  udp.listen( true );          // Listen for received packets
}

void draw() {
  //PImage img = loadImage("background.png"); // Placeholder Image, change in future
  //background(img);
  r = cp5.get(ColorWheel.class,"c").r();
  g = cp5.get(ColorWheel.class,"c").g();
  b = cp5.get(ColorWheel.class,"c").b();

  refreshCanvas();
  
  
}


// BUTTON FUNCTIONS //
public void A0On(){
  udp.send("c:0:1", ipAddress, udpPort ) ;
  analogChannelOn[0] = true;
}
public void A1On(){
  udp.send("c:1:1", ipAddress, udpPort ) ;
  analogChannelOn[1] = true;
}
public void A2On(){
  udp.send("c:2:1", ipAddress, udpPort ) ;
  analogChannelOn[2] = true;
}
public void A3On(){
  udp.send("c:3:1", ipAddress, udpPort ) ;
  analogChannelOn[3] = true;
}
public void A4On(){
  udp.send("c:4:1", ipAddress, udpPort ) ;
  analogChannelOn[4] = true;
}
public void A5On(){
  udp.send("c:5:1", ipAddress, udpPort ) ;
  analogChannelOn[5] = true;
}
public void A0Off(){
  udp.send("c:0:0", ipAddress, udpPort ) ;
  analogChannelOn[0] = false;
}
public void A1Off(){
  udp.send("c:1:0", ipAddress, udpPort ) ;
  analogChannelOn[1] = false;
}
public void A2Off(){
  udp.send("c:2:0", ipAddress, udpPort ) ;
  analogChannelOn[2] = false;
}
public void A3Off(){
  udp.send("c:3:0", ipAddress, udpPort ) ;
  analogChannelOn[3] = false;
}
public void A4Off(){
  udp.send("c:4:0", ipAddress, udpPort ) ;
  analogChannelOn[4] = false;
}
public void A5Off(){
  udp.send("c:5:0", ipAddress, udpPort ) ;
  analogChannelOn[5] = false;
}
public void RGB_RED_ON(){
  udp.send("p:2:"+String.valueOf(r) , ipAddress, udpPort ) ;
}
public void RGB_GREEN_ON(){
  udp.send("p:0:"+String.valueOf(g), ipAddress, udpPort ) ;
}
public void RGB_BLUE_ON(){
  udp.send("p:1:"+String.valueOf(b), ipAddress, udpPort ) ;
}
public void RGB_RED_OFF(){
  udp.send("p:2:0" , ipAddress, udpPort ) ;
}
public void RGB_GREEN_OFF(){
  udp.send("p:0:0", ipAddress, udpPort ) ;
}
public void RGB_BLUE_OFF(){
  udp.send("p:1:0", ipAddress, udpPort ) ;
}
public void RGB_ALL_ON(){
  udp.send("p:2:"+String.valueOf(r) , ipAddress, udpPort ) ;
  udp.send("p:0:"+String.valueOf(g), ipAddress, udpPort ) ;
  udp.send("p:1:"+String.valueOf(b), ipAddress, udpPort ) ;
}

public void RGB_ALL_OFF(){
  udp.send("p:0:0", ipAddress, udpPort ) ;
  udp.send("p:1:0", ipAddress, udpPort ) ;
  udp.send("p:2:0", ipAddress, udpPort ) ;
}

public void RGB_RED(){
  udp.send("p:2:255" , ipAddress, udpPort ) ;
  udp.send("p:0:0", ipAddress, udpPort ) ;
  udp.send("p:1:0", ipAddress, udpPort ) ;
}
public void RGB_GREEN(){
  udp.send("p:0:255", ipAddress, udpPort ) ;
  udp.send("p:1:0", ipAddress, udpPort ) ;
  udp.send("p:2:0", ipAddress, udpPort ) ;
}
public void RGB_BLUE(){
  udp.send("p:1:255", ipAddress, udpPort ) ;
  udp.send("p:0:0", ipAddress, udpPort ) ;
  udp.send("p:2:0", ipAddress, udpPort ) ;
}
public void RGB_YELLOW(){
  udp.send("p:2:255", ipAddress, udpPort ) ;
  udp.send("p:0:255", ipAddress, udpPort ) ;
  udp.send("p:1:0", ipAddress, udpPort ) ;
}
public void Y_LED_ON(){
  udp.send("d:15:h", ipAddress, udpPort ) ;
}
public void Y_LED_OFF(){
  udp.send("d:15:l", ipAddress, udpPort ) ;
}
public void R_LED_ON(){
  udp.send("d:5:h", ipAddress, udpPort ) ;
}
public void R_LED_OFF(){
  udp.send("d:5:l", ipAddress, udpPort ) ;
}
public void G_LED_ON(){
  udp.send("d:4:h", ipAddress, udpPort ) ;
}
public void G_LED_OFF(){
  udp.send("d:4:l", ipAddress, udpPort ) ;
}
public void READ_D0(){
  udp.send("r:0", ipAddress, udpPort ) ;
  toDigitalRead[0] = true;
}
public void READ_D1(){
  udp.send("r:1", ipAddress, udpPort ) ;
  toDigitalRead[1] = true;
}
public void READ_D2(){
  udp.send("r:2", ipAddress, udpPort ) ;
  toDigitalRead[2] = true;
}
public void READ_D3(){
  udp.send("r:3", ipAddress, udpPort ) ;
  toDigitalRead[3] = true;
}


//Receive function for UDP packets -- TODO

void receive( byte[] data, String ip, int port ) { 
  
  String message = new String( data );
  
  // Print the result
  println( "receive: \""+message+"\" from "+ip+" on port "+port );
  
  for (int i = 0; i <numDigitalInputs; i++) {
   if(toDigitalRead[i] == true && message.substring(0,2).equals("D"+String.valueOf(i))) {
     toDigitalRead[i] = false;
     digitalReads[i] = new String(message);
   }
  }
  
  for (int i = 0; i < numAnalogInputs; i++) {
    if (analogChannelOn[i] == true && message.substring(0,2).equals("A"+String.valueOf(i))) {
      analogReads[i] = new String(message.substring(4));
    }
  }
}

void refreshCanvas() {
  background(0xA9A9A9);
  for (int i = 0; i < numDigitalInputs; i++) {
   text(digitalReads[i], width/2 + i*(width/2)/4, 3*height/8 - pixelOffset, width/8, height/4 ) ;
  }
  
  for (int i = 0; i < numAnalogInputs; i++) {
    text(analogReads[i], width/2 + width/8, height/2 + (i+1)*(height/2)/7 + pixelOffset, (width/2)/4, (height/2)/7 );
  }
  
  text("Pins"    , width/2,                 height/2  + pixelOffset, (width/2)/4 , (height/2)/5 );
  text("Values"  , width/2 + 1*(width/2)/4, height/2  + pixelOffset, (width/2)/4 , (height/2)/5 );
  text("On"      , width/2 + 2*(width/2)/4, height/2  + pixelOffset, (width/2)/4 , (height/2)/5 );
  text("Off"     , width/2 + 3*(width/2)/4, height/2  + pixelOffset, (width/2)/4 , (height/2)/5 );
  
  for (int i = 0; i <= 7; i++) {
   line(width/2, height/2 + i*(height/2)/7, width, height/2 + i*(height/2)/7);
  }
  for (int i = 0; i <= 4; i++) {
   line(width/2 + i*(width/2)/4, height/2, width/2 + i*(width/2)/4, height); 
  }
  for (int i = 0; i <= 6; i++) {
    text("A" + String.valueOf(i) , width/2, height/2 + (i+1)*(height/2)/7 + pixelOffset, (width/2)/4, (height/2)/7 );
  }
  
}
