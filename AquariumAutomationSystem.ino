#include <ThingSpeak.h>
#include <Servo.h>
#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"
//EthernetConnection
// Local Network Settings
byte mac[] = { 0xA8, 0x77, 0xE5, 0xA3, 0xE1, 0x89 };  // Must be unique on local network
byte ip[] = { 192, 168, 0, 150 };                     // Must be unique on local network
byte gateway[] = { 192, 168, 1, 1 };
byte subnet[] = { 255, 255, 255, 0 };

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String readAPIKey = "FDCO8QNI3828JGHJ";  // Write API Key for a ThingSpeak Channel
const char* writeAPIKey = "SCF1D880TOYSXH56";
const int updateInterval = 10000;  // Time interval in milliseconds to update ThingSpeak
unsigned long channel = 2171425;

// Initialize Arduino Ethernet Client
EthernetClient client;

//Light System
const int pResistor = A0;  // Photoresistor at Arduino analog pin A0
const int ledPin = 5;      // Led pin at Arduino pin 9
int lightValue;            // Store value from photoresistor (0-1023)

//pH monitoring System
float calibration_value = 5;
const int phPin = A1;
int phVal;

//Temperature System
#define DHTPIN 3     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//Feeder System
Servo myservo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position
int feed;
void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip, gateway, subnet);
  delay(1000);
  Serial.print("ETHERNET SHIELD ip  is     : ");
  Serial.println(Ethernet.localIP());
  // Start Ethernet on Arduino
  startEthernet();
  ThingSpeak.begin(client);

  pinMode(ledPin, OUTPUT);
  pinMode(pResistor, INPUT);

  dht.begin();

  myservo.attach(8);  // attaches the servo on pin 9 to the servo object
  Serial.print("serve: ");
  Serial.println(myservo.attached());
}

void loop() {
  //Controlling led based on ambient light detected using photoresistor
  lightValue = analogRead(pResistor);
  if (lightValue > 25) {
    digitalWrite(ledPin, LOW);                          //Turn led off
    ThingSpeak.writeField(channel, 3, 0, writeAPIKey);  //set lightstate to 0 in the API
  } else {
    digitalWrite(ledPin, HIGH);                         //Turn led on
    ThingSpeak.writeField(channel, 3, 1, writeAPIKey);  //set lightstate to 1 in the API
  }
  checkLightState();

  phVal = analogRead(phPin);
  float volt = (float)phVal * 5.0 / 1024;
  float ph_act = 3.5 * volt - calibration_value;
  ThingSpeak.writeField(channel, 2, ph_act, writeAPIKey);

  float t = dht.readTemperature();
  Serial.println(t);
  ThingSpeak.writeField(channel, 1,t, writeAPIKey);

  //check for feed commands
  feed=ThingSpeak.readFloatField(channel, 4);
  Serial.print(feed);
  if(feed==1)
    feedFish();

  delay(3000);
}
void startEthernet() {
  client.stop();
  Serial.println("Connecting Arduino to network…");
  Serial.println();
  delay(1000);

  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  } else {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
    Serial.println("Data being uploaded to THINGSPEAK Server…….");
    Serial.println();
  }

  delay(1000);
}
//check for user's commands to turn on lights
void checkLightState() {
  int lightState = ThingSpeak.readFloatField(channel, 3);

  if (lightState == 1) {
    digitalWrite(ledPin, HIGH);
    Serial.println("turning on the lights");
  } else {
    digitalWrite(ledPin, LOW);
    Serial.println("turning the lights off");
  }
}

//move the servo motor
void feedFish(){
   for (pos = 0; pos <= 160; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
  delay(2000);
  for (pos = 160; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15 ms for the servo to reach the position
  }
   ThingSpeak.writeField(channel, 4, 0, writeAPIKey);
    delay(2000);
}
