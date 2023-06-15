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
//OLED Setup
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
  delay(200);
  WiFiDrv::digitalWrite(25, LOW); //GREEN
  
  WiFiDrv::pinMode(26, OUTPUT); //Red
  WiFiDrv::digitalWrite(26, HIGH);   //RED
  delay(200);
  WiFiDrv::digitalWrite(26, LOW);   //RED

  WiFiDrv::pinMode(27, OUTPUT); //Blue
  WiFiDrv::digitalWrite(27, HIGH);   //BLUE
  delay(200);
  WiFiDrv::digitalWrite(27, LOW);   //BLUE

  delay(200);

  WiFiDrv::digitalWrite(25, HIGH); //GREEN
  WiFiDrv::digitalWrite(26, HIGH);   //RED
  WiFiDrv::digitalWrite(27, HIGH);   //BLUE
  delay(200);
  WiFiDrv::digitalWrite(25, LOW); //GREEN
  WiFiDrv::digitalWrite(26, LOW);   //RED
  WiFiDrv::digitalWrite(27, LOW);   //BLUE
}

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void oledSetup() {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

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

void netSetup() {
  //Try to connect to NB 2.4G
  while (status != WL_CONNECTED) {

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("AEL Booting|-      |");
    display.println("Connecting to:");
    display.println(ssid);
    display.display();

    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    WiFiDrv::digitalWrite(25, HIGH); //GREEN
    WiFiDrv::digitalWrite(26, HIGH);   //RED
    delay(100);
    WiFiDrv::digitalWrite(25, LOW); //GREEN
    WiFiDrv::digitalWrite(26, LOW);   //RED
    delay(5000);
  }

  //Show lights to indicate successful connection

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

  WiFiDrv::digitalWrite(25, HIGH); //GREEN
  WiFiDrv::digitalWrite(27, HIGH); //BLUE
  delay(500);
  WiFiDrv::digitalWrite(25, LOW); //GREEN
  WiFiDrv::digitalWrite(27, LOW); //BLUE
  delay(4000);
}

void envSetup() {
  //init the ENV Shield//
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
    display.println("Failed to initialize\nSD Card!");
    display.display();
    while (1);
  display.println("SD Card Found");
  display.display();
  delay(1000);
  }
}

void webServer() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");
    // an HTTP request ends with a blank line
    WiFiDrv::digitalWrite(26, HIGH);   //RED
    WiFiDrv::digitalWrite(27, HIGH);   //BLUE
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
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
    Serial.println("client disconnected");
    WiFiDrv::digitalWrite(26, LOW);   //RED
    WiFiDrv::digitalWrite(27, LOW);   //BLUE
  }
}


void setup() {
  //LED setup
  ledSetup();

  //OLED setup
  oledSetup();

  //WiFi Network Setup
  netSetup();

  //Init the ENV Sheild
  envSetup();
  

  //Start NTP client
  timeClient.begin();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AEL Booting|----   |\nStarting NTP Client");
  display.display();
  delay(500);

  // init SPI
  SPI.begin();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AEL Booting|-----  |\nStarting SPI");
  display.display();
  delay(500);

  //SD card setup
  sdSetup();

  // init the logfile
  dataFile = SD.open("testing.csv", FILE_WRITE);
  delay(1000);
  // init the CSV file with headers
  dataFile.println("Time,Temperature,Humidity,Light,UVA,UVB,UVI");
  // close the file
  dataFile.close();
  //White LED
  
  //Start Web server
  server.begin();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AEL Booted |-------|");
  display.display();

  WiFiDrv::digitalWrite(26, HIGH);   //RED
  WiFiDrv::digitalWrite(27, HIGH);   //BLUE
  WiFiDrv::digitalWrite(25, HIGH); //GREEN
  delay(1000);
  WiFiDrv::digitalWrite(25, LOW); //GREEN
  WiFiDrv::digitalWrite(26, LOW);   //RED
  WiFiDrv::digitalWrite(27, LOW);   //BLUE
  delay(1000);


}
//END OF SETUP//



void loop() {
  WiFiDrv::digitalWrite(26, HIGH); //RED
  timeClient.update();
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
    WiFiDrv::digitalWrite(27, HIGH); //BLUE
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
    

    // open the logfile
    dataFile = SD.open("Benji.csv", FILE_WRITE);

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
    count2 = count2 + 1;
  }
  count2 = 0;
}