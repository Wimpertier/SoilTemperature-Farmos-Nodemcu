#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <time.h>
#include <TimeLib.h>

#define ONE_WIRE_BUS D1

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
float temperatur;

const char* ssid     = "your ssid";
const char* password = "your password";

String publicKey = "public key from Farmos for this sensor";
String privateKey = "private key from Farmos for this sensor";
String thumbprint = "thumbprint/fingerprint from ssl certicate"; //Note the certificates experation date!! You have to update the thumbprint/fingerprint after this date.

int timeZone = 0; //difference from UTC in hours
int getMinutes; 
//int timeInterval = 5; //set time interval in minutes

void connectWifi() {
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.println("Waiting for connection...");
  }
}

void receiveTime() {
  
  configTime(timeZone * 3600, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) <= 100000) {

    delay(500);
    Serial.println("Waiting for time...");
  }
}

void setup() {

  Serial.begin(9600);
 
  sensor.begin();

  connectWifi();
  receiveTime();  
}

void loop() {

  if (WiFi.status() == WL_CONNECTED) {

    time_t timeNow = time(nullptr);  
    getMinutes = minute(timeNow);
  
    if (getMinutes == 0) {
    // if (getMinutes == 0)  -> send the data hourly to Farmos
    // if (getMinutes % timeInterval == 0) -> send the data in given time intervals, set in minutes, see above for the variable int timeInterval = 5; (time interval 5 minutes)   

      sensor.requestTemperatures(); 
      temperatur = sensor.getTempCByIndex(0); //sensor.getTempFByIndex(0) for Fahrenheit

      Serial.print("Temperature: ");
      Serial.println(temperatur);

      HTTPClient https; 
      https.setTimeout(5000);
      
      https.begin("https://www.example.com/farmos/farm/sensor/listener/" + publicKey + "?private_key=" + privateKey, thumbprint);  
         
      https.addHeader("Content-Type", "application/json");   
      String message = "{\"timestamp\":" + String(timeNow) + ",\"Soil temperature\":" + String(temperatur) + "}";
      Serial.println(message);
      int httpCode = https.POST(message);                      
      
      if (httpCode == -1) {
        
        Serial.println("Connection refused from the server!");
      }
      else if (httpCode == 200) {
        
        Serial.println("Succeeded!");
      }
      else {
        
        Serial.print("Something went wrong. Here is the code: ");
        Serial.print((httpCode)
        Serial.println(".");
      }
     
      https.end();

      //delay(60000); //run once per minute

      ESP.deepSleep(3420000000);  //deep sleep in 57 minutes   
    }   
  } 
  else {

    connectWifi();
    receiveTime();       
  }
}
