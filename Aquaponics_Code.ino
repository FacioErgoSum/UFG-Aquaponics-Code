//libraries for Ethernet
#include <SPI.h>
#include <Ethernet.h>

// libraries for Character LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// libraries for Temperature Sensor
#include <Wire.h>
#include "SparkFunHTU21D.h"

// libraries for RTC
#include <virtuabotixRTC.h>

// libraries for Water Temp. Sensor
#include <OneWire.h> 
#include <DallasTemperature.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

// set pins for LCD
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

//Create an instance of the temperature sensor object
HTU21D myHumidity;

// Creating the RTC Object
virtuabotixRTC myRTC(6, 7, 8);

//defines pins for Water Temp Sensor
#define ONE_WIRE_BUS 9 

//creates a OneWire instance
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

//variables for pH sensor
#define SensorPin 0          //pH meter Analog output to Arduino Analog Input 0
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

// defines pins numbers for UltraSonic Sensor
const int trigPin = 2;
const int echoPin = 3;

// defines UltraSonic variables
long duration;
int distance;

void setup() 
{
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  pinMode(13,OUTPUT); //pH sensor pin
  
  Serial.begin(9600); // used for printing to the Serial Monitor

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  // Starts Water Temp sensor 
  sensors.begin();

  myHumidity.begin(); // start measuring humidity

  // initialize the lcd for 20 chars 4 lines and turn on backlight
  lcd.begin(20,4); 
  lcd.backlight(); 

  //RTC time setup (s, m, h, d of week, d of month, m, y)
  myRTC.setDS1302Time(00, 47, 4, 2, 10, 7, 2017);

}


void loop()   
{
  /**************************PRINTS PH SENSOR VALUES*****************************/
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value
  Serial.print("pH:");  
  Serial.print(phValue,2);
  Serial.println(" ");
  digitalWrite(13, HIGH);       
  delay(800);
  digitalWrite(13, LOW); 

  /**************************CREATES TEMP AND HUMD VARIABLES*****************************/
  
  // variables for temperature and humidity
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();

  // convert Celsius to Fahrenheit 
  float tempf = temp * 1.8 + 32;

/**************************PRINTS TEMPERATURE AND HUMIDITY*****************************/
  // print variables and values
  Serial.print("Time:");
  Serial.print(millis());
  Serial.print(" Temp:");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print(" Humd:");
  Serial.print(humd, 1);
  Serial.print("%");
  Serial.println();

/***************************PRINTS TIME FROM RTC***********************************/
  // Keeps time updated, and also allows for grabbing of elements (seconds, day, year)
  myRTC.updateTime(); 

  // Start printing time elements as individuals 
  Serial.print("Current Date / Time: "); 
  Serial.print(myRTC.dayofmonth); 
  Serial.print("/"); 
  Serial.print(myRTC.month); 
  Serial.print("/");
  Serial.print(myRTC.year);
  Serial.print(" ");
  Serial.print(myRTC.hours);
  Serial.print(":");
  Serial.print(myRTC.minutes);
  Serial.print(":");
  Serial.println(myRTC.seconds);

/*****************************PRINTS WATER TEMPERATURE**********************************/
  Serial.print(" Requesting temperatures..."); 
  sensors.requestTemperatures(); // Send the command to get temperature readings 
  Serial.println("DONE"); 
  Serial.print("Temperature is: "); 
  Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"?  
  // You can have more than one DS18B20 on the same bus.  
  // 0 refers to the first IC on the wire 
  // convert Celsius to Fahrenheit 
  float wtemp = sensors.getTempCByIndex(0) * 1.8 + 30;
  Serial.println();

/*******************************PRINTS DEPTH****************************************/
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);


/******************************WRITES VALUES TO LCD**************************************/
  // write characters to LCD
  // (NOTE: Cursor Position: CHAR, LINE) 
  lcd.clear(); 
  lcd.setCursor(3,0); //Start at character 4 on line 0
  lcd.print("UFG Aquaponics");
  lcd.setCursor(0,1);
  lcd.print("Air Temp: ");
  lcd.print(tempf);
  lcd.print("F");  
  lcd.setCursor(0,2);
  lcd.print("Water Temp: ");
  lcd.print(wtemp);
  lcd.print("F"); 
  lcd.setCursor(0,3);   
  lcd.print("Humidity: ");
  lcd.print(humd);
  lcd.print("%");
  delay(7000);
  lcd.setCursor(0,1); //Start at character 4 on line 0
  lcd.print("                    ");
  lcd.setCursor(0,1);
  lcd.print("Time: ");
  lcd.print(myRTC.hours);
  lcd.print(":");
  lcd.print(myRTC.minutes);
  lcd.print(":");
  lcd.print(myRTC.seconds);
  lcd.setCursor(0,2);
  lcd.print("                    ");
  lcd.setCursor(0,2);
  lcd.print("Distance: ");
  lcd.print(distance);
  lcd.print("cm");
  lcd.setCursor(0,3);
  lcd.print("                    ");
  lcd.setCursor(0,3);
  lcd.print("pH Level: ");
  lcd.print(phValue,2);
  delay(7000);
  
  //*************************WEB SERVER CODE****************************//
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          //********************WRITE TEXT OUTPUT HERE************************//
          client.println("Temperature: ");
          client.println(tempf);
          client.println("<br />");
          client.println("Humidity: ");
          client.println(humd);
          client.println("<br />");
          client.println("Water Temperature: ");
          client.println(wtemp);
          client.println("<br />");
          client.println("pH Level: ");
          client.println(phValue,2);
          client.println("<br />");
          client.println("Depth: ");
          client.println(distance);
          client.println("<br />");
          client.print("Time: ");
          client.print(myRTC.hours);
          client.print(":");
          client.print(myRTC.minutes);
          client.print(":");
          client.print(myRTC.seconds);
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
  }  
  
}
