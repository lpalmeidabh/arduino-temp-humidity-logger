#include <Ethernet.h>
#include <SPI.h>
#include "DHT.h"
#include <stdlib.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);


//Ethernet
byte mac[] = { 0xFF, 0x91, 0x37, 0xA2, 0xBC, 0xD2 }; 
byte ip[] = { 192, 168, 0, 177 };
byte server[] = { 192, 168, 0, 11 };
int port = 3000;

//char server[] = "arduino-temp-humidity-logger.herokuapp.com";
//int port = 80;
byte gateway[] = { 192, 168, 0, 1 };
byte subnet[] = { 255, 255, 255, 0 };

EthernetClient client;

void setup() {
  Serial.begin(9600);
  setupEthernet();
  dht.begin();
}

void loop() {
  delay(500000); 
  
  updateTH();    
}

void updateTH(){

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();

  // Read temperature as Celsius
  float t = dht.readTemperature();

  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Compute heat index
  // Must send in temp in Fahrenheit!
  float hi = dht.computeHeatIndex(f, h);
  float hi_c = (hi-32)*(5/9);

  char tmp[10];
  String data = "thlog[temperature_celsius]=";
  data = data + dtostrf(t,1,2,tmp);

  data = data + "&thlog[temperature_fahrenheit]=";
  data = data + dtostrf(f,1,2,tmp);
  data = data + "&thlog[humidity]=";
  data = data + dtostrf(h,1,2,tmp);
  data = data + "&thlog[heat_index_celsius]=";
  data = data + dtostrf(hi_c,1,2,tmp);
  data = data + "&thlog[heat_index_fahrenheit]=";
  data = data + dtostrf(hi,1,2,tmp);
  
  int data_length = data.length();
  Serial.println("Establishing connection...");
     
  if (client.connect(server, port)) {

    Serial.println("\n\nConnected");
    client.println("POST /th_logs HTTP/1.1");
    client.println("Host: arduino-temp-humidity-logger.herokuapp.com");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.println(data);

    //DEBUG
    Serial.println("POST /th_logs HTTP/1.1");
    Serial.println("Host: arduino-temp-humidity-logger.herokuapp.com");
    Serial.println("User-Agent: Arduino/1.0");
    Serial.println("Connection: close");
    Serial.println("Cache-Control: no-cache");
    Serial.println("Content-Type: application/x-www-form-urlencoded");
    Serial.print("Content-Length: ");
    Serial.println(data.length());
    Serial.println();
    Serial.print(data);
  }
  else{
    Serial.println("Connection failed");
  }
  
  if (client.connected()) {
    Serial.println();
    Serial.println("\n\ndisconnecting.");
    client.stop();
  }

}

void setupEthernet()
{
  Serial.println("Setting up Ethernet...");
  Ethernet.begin(mac, ip, gateway, subnet);
  Serial.println("My IP address: ");
  Serial.println(Ethernet.localIP());
}

