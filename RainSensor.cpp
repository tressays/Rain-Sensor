/* 
 * Project "RammedEarth Rain Sensor"
 * Author:Crowhill
 * Date: 20-Nov-2023
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */
#include "Particle.h"
#include "Adafruit_SSD1306.h"
#include "Credentials.h"
#include "IoTClassroom_CNM.h"
#include "Adafruit_BME280.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_SPARK.h"
#define OLED_RESET D4
TCPClient TheClient; 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);
Adafruit_MQTT_Publish pubRain = Adafruit_MQTT_Publish(&mqtt,"RammedEarth/rain");
Adafruit_MQTT_Publish pubTempF = Adafruit_MQTT_Publish(&mqtt,"RammedEarth/TempF");
Adafruit_MQTT_Publish pubStorm = Adafruit_MQTT_Publish(&mqtt,"RammedEarth/Storm");
Adafruit_MQTT_Publish pubHumid = Adafruit_MQTT_Publish(&mqtt,"RammedEarth/Humid");
String DateTime, TimeOnly;
const int Bled = D3;
int outputValue; //4095 means maximum dryness, reads red on the gauge
int sensorPin = A0;
int sensorValue;
unsigned int last, lastTime;
int bStatus;                            //BME_280
int TempC, TempF;
int Press;
int Altit;
int Humid;
Adafruit_BME280 BME;
Adafruit_SSD1306 display(OLED_RESET);
float rainSensor;
float Storm;
void MQTT_connect();
bool MQTT_ping();
bool publish();
SYSTEM_MODE(SEMI_AUTOMATIC); //CHANGE TO AUTOMATIC TO CONNECT TO MQTT


void setup(){


  Serial.begin(9600);
   WiFi.on();
  WiFi.connect();
  while(WiFi.connecting());
  
  pinMode(Bled, OUTPUT);
  digitalWrite(Bled, LOW);
  bStatus = BME.begin(0x76); //BME addr
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize OLED with the I2C addr 0x3D (for the 128x64)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  delay(1000);
  display.clearDisplay();

}
void loop() {
 MQTT_connect();
int sensorValue = analogRead(sensorPin);  // Read the analog value from sensor
int outputValue = map(sensorValue, 0, 4095, 0, 255); // map the 10-bit data to 8-bit data
analogWrite(Bled, outputValue); // generate PWM signal
  //return outputValue;             // Return analog rain value
  //Serial.printf("%i\n",sensorValue);
  //delay(500);
     rainSensor =sensorValue;


  
    TempC = BME.readTemperature(); 
    TempF = map(TempC, 0, 100, 32, 212);    //Celcius to Farenhight
    //Serial.printf("%i\n", Temp);
    delay(1000);
    Press = BME.readPressure();
    //Serial.printf("%ihPa\n", Press); //Pascals(hps)101,325 Pa (1,013.25 hPa), which is equivalent to 1,013.25 millibars,[1] 760 mm Hg, 29.9212 inches Hg, or 14.696 psi.
    delay(1000);
    Humid = BME.readHumidity();
    //Serial.printf("%i\n", Humid);
    delay(1000);
    
    if(Press <1619){
      digitalWrite(Bled, HIGH);
    }
      else{
      (digitalWrite(Bled, LOW));
    }
  
     
   if((millis()-lastTime) > 6000) {   //BME_PUBLISH
    if(mqtt.Update()) {
      Storm =Press;
      pubRain.publish(rainSensor);
      pubTempF.publish(TempF);
      pubStorm.publish(Storm);
      pubHumid.publish(Humid);
    Serial.printf("Publishing %i \n", Storm); //Albuquerqes average =1619/ p = 101325 (1 - 2.25577 x 10^-5 x 1619)^5.25588 p = 83470.6 Pa
      } 
    lastTime = millis();
  }
  
  display.clearDisplay(); 
  display.setCursor(0,0); //
  display.printf("Temperature %i",TempF);
  display.setCursor(0,15); 
  display.printf("hPa %i", Press);
  display.setCursor(0,30); 
  display.printf("Humidity %i",Humid);
  display.setCursor(0,45);
  display.printf("Precip less than 4095 %i", outputValue);
  display.display();
}
 
void MQTT_connect() {
  int8_t ret;
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}





 

