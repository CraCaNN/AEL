# AEL
**Arduino Environment Logger**

AEL is a simple Arduino program which is designed to run on the MKR WIFI 1010 with the MKR ENV Shield.

AEL can log Temperature, Humidity, Pressure, Light, UVA, UVB, UV Index and time to a CSV file.

AEL will log a datapoint roughly every 10 seconds. However, due to the way I have programmed this it will vary (around 1 second over a minute) but timestamps are next to each logged datapoint.

Time is found using NTP through a WiFi connection. WiFi can be configured through arduino_secrets.h.

AEL can also be accessed though a browser by going to the device IP address.

AEL is also able to output all data to an SSD1306 128x32 OLED screen.

AEL shows its startup process through the OLED screen and the LED onboard the MKR1010

## Dependancies
All libaries can be found in the arduino libary manager

Arduino_MKRENV - 
Read data from the MKR ENV Sheild

NTPClient - 
Get time over the network

SD - 
Read/Write to SD cards

WiFiNINA - 
Use the WIFI on MKR boards

Adafruit BusIO

Adafruit GFX Libary - 
Creating text for the display

Adafruit SSD1306 - 
Writing to the display

## Known Issues
I created this program and thought it was cool enough to upload to github so there will be bugs but I may improve it if i have spare time

### Unsolvable Issues
For some reason if the humidity sensor is brought into the sun the sensor readings go negative. This is a hardware issue that I have found other people having as well. 
I may code the sensor to read a minimum value of 0 that way any graphs created later don't look like garbage.

### Solvable Issues

If you do not have a WiFi connection the program will not start. This is because NTP requires WiFi to get time and also update occasionally. If you loose connection after starting the program, it will freeze trying to update time. 

If you are missing the OLED screen the program will not start. I will try and fix this at some point.

If you are missing an SD card the program will not start. I may change it to act more like a weather station so an SD card is not required to start.

If you have the newer MKR ENV sheild this does not have the UV sensor it will probably break something or you will just get blank datapoints for those sensors. I can't test this.
