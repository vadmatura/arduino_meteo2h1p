/*
git clone https://github.com/adafruit/DHT-sensor-library.git
git clone https://github.com/sparkfun/BMP180_Breakout_Arduino_Library.git
*/
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <DHT.h>
#include <SFE_BMP180.h>
#include <LiquidCrystal.h>

#define DHT22_PIN0   10
#define DHT22_PIN1   9
#define MY_BMP180_PRESSURE_ERROR_MMHG 6
//SDA A4
//SDL A5

#define DHTTYPE     DHT22 //DHT 22 (AM2302)
DHT dht0(DHT22_PIN0, DHTTYPE);
DHT dht1(DHT22_PIN1, DHTTYPE);
SFE_BMP180 bmp180;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // (RS, E, DB4, DB5, DB6, DB7);
volatile int time0 = 0, time1 = 0;

//custom symbols
/*byte celsiumGrad[8] = {
  B10111,
  B01011,
  B10111,
  B11100,
  B11011,
  B11011,
  B11100,
  B11111
};*/
byte celsiumGrad[8] = {
  B01000,
  B10100,
  B01000,
  B00011,
  B00100,
  B00100,
  B00011
};
byte milliMeter[8] = {
  B10001,
  B11011,
  B10101,
  B00000,
  B10001,
  B11011,
  B10101,
  B00000
};
byte hidrarhium[8] = {
  B10100,
  B11100,
  B10100,
  B00011,
  B00100,
  B00101,
  B00011,
  B00000
};
/*byte invPercent[8] = {
  B00111,
  B00110,
  B11101,
  B11011,
  B10111,
  B01100,
  B11100,
  B11111
};*/

void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  lcd.begin(16, 2);
  lcd.createChar(0, celsiumGrad);
  lcd.createChar(1, milliMeter);
  lcd.createChar(2, hidrarhium);
  //lcd.createChar(3, invPercent);

  dht0.begin();
  dht1.begin();
  if (bmp180.begin()) {
    prn0("BMP180 init success");
  } else {
    prn0("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
  delay(2000);
}

void loop() {
  double pressure2, temperature2;
  float humidity0 = dht0.readHumidity();
  float temperature0 = dht0.readTemperature();
  float humidity1 = dht1.readHumidity();
  float temperature1 = dht1.readTemperature();
  char status = bmp180.startTemperature();
  if (status != 0) {
    // Wait for the measurement to complete:
    delay(status);
    status = bmp180.getTemperature(temperature2);
    if (status != 0) {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.
      status = bmp180.startPressure(3);
      if (status != 0) {
        // Wait for the measurement to complete:
        delay(status);
        bmp180.getPressure(pressure2, temperature2);
      } else prn0("error starting pressure measurement\n");
    } else prn0("error retrieving temperature measurement\n");
  } else prn0("error starting temperature measurement\n");

  Serial.print(" ");
  Serial.print(humidity0, 1);
  Serial.print(" ");
  Serial.print(temperature0, 1);
  Serial.print(" ");
  Serial.print(humidity1, 1);
  Serial.print(" ");
  Serial.print(temperature1, 1);
  Serial.print(" ");
  Serial.print(pressure2, 1);
  Serial.print(" ");
  Serial.print(temperature2, 1);
  Serial.println("");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("     *|  %|  /  ");
  lcd.setCursor(5, 0);
  lcd.write(byte(0));
  lcd.setCursor(6, 0);
  lcd.print((char)0xFF);
  lcd.setCursor(10, 0);
  lcd.print((char)0xFF);
  lcd.setCursor(0, 1);
  lcd.print("     *|  %|     ");
  lcd.setCursor(5, 1);
  lcd.write(byte(0));
  lcd.setCursor(6, 1);
  lcd.print((char)0xFF);
  lcd.setCursor(10, 1);
  lcd.print((char)0xFF);
  lcd.setCursor(11, 1);
  lcd.write(byte(1));
  lcd.setCursor(12, 1);
  lcd.write(byte(2));

  lcd.setCursor(0, 0);
  lcd.print((temperature0 + temperature2) / 2, 1);
  lcd.setCursor(0, 1);
  lcd.cursor();
  lcd.print(temperature1, 1);

  lcd.setCursor(7, 0);
  lcd.print(round(humidity0));
  lcd.setCursor(7, 1);
  lcd.print(round(humidity1));

  lcd.setCursor(11, 0);
  lcd.print(time0);
  lcd.setCursor(14, 0);
  lcd.print(time1);
  lcd.setCursor(13, 1);
  lcd.print(round(mb2mmhg(pressure2)));

  delay(5000);  // Pause for 5 seconds.

  if (Serial.available() > 0) {
    time0 = Serial.parseInt();
    time1 = Serial.parseInt();
  }
}

void prn0(const char* s) {
  Serial.println(s);
  lcd.setCursor(0, 0);
  lcd.print(s);  
}

double mb2mmhg(double mb) {
  return (mb * 0.750061683) - MY_BMP180_PRESSURE_ERROR_MMHG;
}


/*
#!/usr/bin/python
#sudo apt-get install python-serial python3-serial
#nohup python ./bus22.py &
import json
import urllib2
import serial
import time

url = "https://city.dozor.tech/data?t=3&p=12193,11913,11729,12046,12131,19604,11436,9231"
cookie = "gts.web.uuid=7BDFE4CB-B054-42CA-A607-6FF285CA2B29; gts.web.city=iv-frankivsk; gts.web.google_map.center.lon=48.92991499319314; gts.web.google_map.center.lat=24.71123456954956; gts.web.google_map.zoom=16"

serport = serial.Serial("/dev/ttyUSB0", 9600, timeout=1)

while True:
  while serport.inWaiting() > 0:
    sensors = serport.read()
    print sensors
    r = urllib2.Request(url, headers={'Cookie':cookie})
    response = urllib2.urlopen(r)
    data = json.loads(response.read())
    a1 = data["data"]["a1"]
    timeS = [0,0,0,0]
    timePos = 0
    for a in a1:
      if a["rId"] == 843:
        print(a["t"])
        timeS[timePos] = a["t"]
        timePos = timePos + 1
    strTimeS = "{} {}\n".format(timeS[0], timeS[1])
    print(strTimeS)
    serport.write(strTimeS)
    time.sleep(2)
*/
