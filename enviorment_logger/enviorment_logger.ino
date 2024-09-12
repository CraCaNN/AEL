/*
Copyright 2024 CraCaN

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//Configurable Settings

//Set to true to show more information on OLED 
bool moreInfo = false;

//Enter name of the file to be written to the SD card
String logName = "AEL.csv";



//Getting time through NTP
#include <NTPClient.h>
//Wifi Module
#include <WiFiNINA.h>
//UDP for NTP Time
#include <WiFiUdp.h>

//MKR ENV Shield
#include <Arduino_MKRENV.h>
//SPI
#include <SPI.h>
//SD
#include <SD.h>
//WiFi Details
#include "wifiDetails.h"
//For SSD OLED
#include <Wire.h>
//GFX Libary
#include <Adafruit_GFX.h>
//SSD OLED 1306 Libary
#include <Adafruit_SSD1306.h>

//OLED paramaters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

//For loops
int count = 0;
int count2 = 0;

// chip select for SD card
const int SD_CS_PIN = 4;  

// variables
String ntpftime = "";

//Temp reading variable
float pressure = 0;
float temperature = 0;
float humidity = 0;
float LXLight = 0;
//UV Variables
float uvA = 0;
float uvB = 0;
float uvI = 0;

//HTTP Port
WiFiServer server(80);

// file object
File dataFile;

//get ssid and password for wifi
char ssid[] = FSSID;
char pass[] = FPASS;
int status = WL_IDLE_STATUS;// the WiFi radio's status

//setup ntp udp stuff
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void ledSetup() {
  //Onboard LEDs setup
  WiFiDrv::pinMode(25, OUTPUT); //Green
  WiFiDrv::digitalWrite(25, HIGH); //GREEN
  delay(100);
  WiFiDrv::digitalWrite(25, LOW); //GREEN
  
  WiFiDrv::pinMode(26, OUTPUT); //Red
  WiFiDrv::digitalWrite(26, HIGH);   //RED
  delay(100);
  WiFiDrv::digitalWrite(26, LOW);   //RED

  WiFiDrv::pinMode(27, OUTPUT); //Blue
  WiFiDrv::digitalWrite(27, HIGH);   //BLUE
  delay(100);
  WiFiDrv::digitalWrite(27, LOW);   //BLUE

  WiFiDrv::digitalWrite(25, HIGH); //GREEN
  WiFiDrv::digitalWrite(26, HIGH);   //RED
  WiFiDrv::digitalWrite(27, HIGH);   //BLUE
  delay(100);
  WiFiDrv::digitalWrite(25, LOW); //GREEN
  WiFiDrv::digitalWrite(26, LOW);   //RED
  WiFiDrv::digitalWrite(27, LOW);   //BLUE
}

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void oledSetup() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // Clear the buffer
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  Serial.println("clear display");
  
  display.clearDisplay();
  
  Serial.println("draw pixel");
  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  Serial.println("OLED Initialised");
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("AEL Booting|       |");
  display.display(); 
  delay(200);
}

void cDis(){//Clear display and buffer
  display.clearDisplay();
  display.setCursor(0, 0);
}

bool hasNet = false;
void netSetup() {
  //Try to connect to Network
  while (status != WL_CONNECTED) {
    cDis();
    display.println("AEL Booting|-      |\nConnecting to:");
    display.println(ssid);
    display.print("Attempt ");
    display.print(count+1);
    display.print("/5");
    display.display();

    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    
    status = WiFi.begin(ssid, pass);// Connect to WPA/WPA2 network:
    WiFiDrv::digitalWrite(25, HIGH); //GREEN
    WiFiDrv::digitalWrite(26, HIGH);   //RED
    delay(100);
    WiFiDrv::digitalWrite(25, LOW); //GREEN
    WiFiDrv::digitalWrite(26, LOW);   //RED
    delay(1000);
    
    Serial.println(status);

    if (count == 4 && status == 4) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Unable to connect to:");
      display.println(ssid);
      display.println("Timestamps inaccurate");
      display.println("Web page unavailable");
      display.display();
      delay(4000);
      break;
    }
    else if (status == 4) {
      pass;
    }
    else {
      IPAddress ip = WiFi.localIP();
      Serial.print("IP Address: ");
      Serial.println(ip);
      Serial.println("You're connected to the network");
      
      //display.clearDisplay();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("AEL Booting|--     |");
      display.println("Connected to WiFi");
      display.print("IP: ");
      display.println(ip);
      display.display();

      hasNet = true;
      //Show lights to indicate successful connection
      WiFiDrv::digitalWrite(25, HIGH); //GREEN
      WiFiDrv::digitalWrite(27, HIGH); //BLUE
      delay(500);
      WiFiDrv::digitalWrite(25, LOW); //GREEN
      WiFiDrv::digitalWrite(27, LOW); //BLUE
      delay(4000);
    }
    count = count + 1;
  }
  count = 0;

  
  
}

void inetTest() {//check that there is still internet before updating NTP time
  //todo 
}

void envSetup() {
  //init the MKR ENV Shield//
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AEL Booting|---    |");
  if (!ENV.begin()) {
    //show red if env shield is not detected
    WiFiDrv::digitalWrite(26, HIGH);   //RED
    Serial.println("Failed to initialize MKR ENV shield!");
    display.println("Failed to initialize\nMKR ENV shield");
    display.display();
    while (1);
  display.println("MKR ENV found");
  display.display();
  delay(1000);
  }
}

bool cardStatus = false;

void sdSetup() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AEL Booting|------ |");
  // init SD card
  if(!SD.begin(SD_CS_PIN)) {
    Serial.println("Failed to initialize SD card!");
    //show purple if sd is not detected
    WiFiDrv::digitalWrite(26, HIGH);   //RED
    WiFiDrv::digitalWrite(27, HIGH);   //BLUE
    display.println("Unable to detect card\nLogging disabled");
    display.display();
    delay(3000);
    WiFiDrv::digitalWrite(26, LOW);   //RED
    WiFiDrv::digitalWrite(27, LOW);   //BLUE
  }
  else {
    display.println("SD Card Found");
    display.display();
    cardStatus = true; //logging is now enabled
  }
}


void webServer() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Web page requested");
    WiFiDrv::digitalWrite(26, HIGH);   //RED
    WiFiDrv::digitalWrite(27, HIGH);   //BLUE


    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);//Print the request
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 60");  // refresh the page automatically
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<title>Arduino Enviorment System</title>");
          client.println("<style>body {background-color: black;} p {color: white; font-family: verdana;}</style>");
          client.println("</head>");
          client.println("<body>");
          client.println("<p>Internal time: ");
          client.println(ntpftime);
          client.println("<br />");
          client.println("<p>Temperature: ");
          client.println(temperature);
          client.println("<br />");
          client.println("Humidity: ");
          client.println(humidity);
          client.println("<br />");
          client.println("Light: ");
          client.println(LXLight);
          client.println("<br />");
          client.println("</p>");
          client.println("<body/>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("Page sent");
    WiFiDrv::digitalWrite(26, LOW);   //RED
    WiFiDrv::digitalWrite(27, LOW);   //BLUE
  }
}


void setup() {
  delay(2000);//delay to allow time for serial to connect
  //LED setup
  ledSetup();
  Serial.println("LED boot complete");

  //OLED setup
  oledSetup();
  Serial.println("Oled initiallised");

  //WiFi Network Setup
  netSetup();
  Serial.println("Network started");
  

  //Init the ENV Sheild
  envSetup();
  Serial.println("Shield started");
  
  
  if (hasNet == true){
    //Start NTP client
    timeClient.begin();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("AEL Booting|----   |\nStarting NTP Client");
    display.display();
    delay(200);
  }
  else {
    timeClient.begin();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("AEL Booting|----   |\nNo Internet\nTime will be\n00:00:00");
    display.display();
    delay(1000);
  }

  // init SPI
  SPI.begin();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AEL Booting|-----  |\nStarted SPI");
  display.display();
  delay(200);

  //SD card setup
  sdSetup();

  if (cardStatus == true) {
  // init the logfile
    dataFile = SD.open(logName, FILE_WRITE);
    delay(1000);
    // init the CSV file with headers
    dataFile.println("Time,Temperature,Humidity,Light,UVA,UVB,UVI");
    // close the file
    dataFile.close();
    //White LED
  }
  
  //Start Web server
  if (hasNet == true){
    server.begin();
    WiFiDrv::digitalWrite(26, HIGH);   //RED
    WiFiDrv::digitalWrite(27, HIGH);   //BLUE
    WiFiDrv::digitalWrite(25, HIGH); //GREEN
    delay(1000);
    WiFiDrv::digitalWrite(25, LOW); //GREEN
    WiFiDrv::digitalWrite(26, LOW);   //RED
    WiFiDrv::digitalWrite(27, LOW);   //BLUE
  }
  else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("AEL Booting |-------|");
    display.println("Skipping webserver start");
    display.display();
    delay(1000);
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AEL Booted |-------|");
  display.display();
  delay(1500);
}
//END OF SETUP//



void loop() {
  WiFiDrv::digitalWrite(26, HIGH); //RED
  timeClient.update();//update time
  WiFiDrv::digitalWrite(26, LOW); //RED

  temperature = ENV.readTemperature();
  humidity = ENV.readHumidity();
  pressure = ENV.readPressure();
  LXLight = ENV.readLux();
  //UV
  uvA = ENV.readUVA();
  uvB = ENV.readUVB();
  uvI = ENV.readUVIndex();

  while (count2 != 10) {
    count = 0;
    
    // read the sensors values
    temperature = ENV.readTemperature();
    humidity = ENV.readHumidity();
    pressure = ENV.readPressure();
    LXLight = ENV.readLux();
    //UV
    uvA = ENV.readUVA();
    uvB = ENV.readUVB();
    uvI = ENV.readUVIndex();

    //format time
    ntpftime = timeClient.getFormattedTime();
    
    if (cardStatus == true) {
      // open the logfile
      dataFile = SD.open("Testing.csv", FILE_WRITE);
      WiFiDrv::digitalWrite(27, HIGH); //BLUE
      // write to sd card each of the sensor values
      dataFile.print(ntpftime);
      dataFile.print(",");
      dataFile.print(temperature);
      dataFile.print(",");
      dataFile.print(humidity);
      dataFile.print(",");
      dataFile.print(LXLight);
      dataFile.print(",");
      dataFile.print(uvA);
      dataFile.print(",");
      dataFile.print(uvB);
      dataFile.print(",");
      dataFile.println(uvI);
      // close the file
      dataFile.close();
      WiFiDrv::digitalWrite(27, LOW); //BLUE
    }

    
    
    while (count != 10) {
      WiFiDrv::digitalWrite(25, HIGH); //GREEN
      webServer();
      WiFiDrv::digitalWrite(25, LOW); //GREEN
      delay(500);
      count = count + 1;
    
      temperature = ENV.readTemperature();
      humidity = ENV.readHumidity();
      pressure = ENV.readPressure();
      LXLight = ENV.readLux();
      //UV
      uvA = ENV.readUVA();
      uvB = ENV.readUVB();
      uvI = ENV.readUVIndex();
    
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Temperature: ");
      display.print(temperature);
      display.println("C");
      display.print("Humidity: ");
      display.print(humidity);
      display.println("%");
      display.print("Pressure: ");
      display.print(pressure);
      display.println("hPa");
      display.print("Light: ");
      display.print(LXLight);
      display.print("LUX");
      display.display();
    }
    count = 0;

    if (moreInfo == true) {
      while (count != 10) {
      ntpftime = timeClient.getFormattedTime();
        WiFiDrv::digitalWrite(25, HIGH); //GREEN
        webServer();
        WiFiDrv::digitalWrite(25, LOW); //GREEN
        delay(500);
        count = count + 1;
      
        temperature = ENV.readTemperature();
        humidity = ENV.readHumidity();
        pressure = ENV.readPressure();
        LXLight = ENV.readLux();
        //UV
        uvA = ENV.readUVA();
        uvB = ENV.readUVB();
        uvI = ENV.readUVIndex();
      
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("UVA: ");
        display.println(uvA);
        display.print("UVB: ");
        display.println(uvB);
        display.print("UVI: ");
        display.println(uvI);
        display.println(ntpftime);
        display.display();
      }
    }
    else {
      while (count != 10) {
        WiFiDrv::digitalWrite(25, HIGH); //GREEN
        webServer();
        WiFiDrv::digitalWrite(25, LOW); //GREEN
        delay(500);
        count = count + 1;
      
        temperature = ENV.readTemperature();
        humidity = ENV.readHumidity();
        pressure = ENV.readPressure();
        LXLight = ENV.readLux();
        //UV
        uvA = ENV.readUVA();
        uvB = ENV.readUVB();
        uvI = ENV.readUVIndex();
      
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Temperature: ");
        display.print(temperature);
        display.println("C");
        display.print("Humidity: ");
        display.print(humidity);
        display.println("%");
        display.print("Pressure: ");
        display.print(pressure);
        display.println("hPa");
        display.print("Light: ");
        display.print(LXLight);
        display.print("LUX");
        display.display();
      }
    count = 0;
    }
    count2 = count2 + 1;
  }
  count2 = 0;
}
