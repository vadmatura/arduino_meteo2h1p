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

#define DHT22_PIN0    10
#define DHT22_PIN1    9
#define LCD_RS        12
#define LCD_E         11
#define LCD_DB4       5
#define LCD_DB5       4
#define LCD_DB6       3
#define LCD_DB7       2
#define MY_BMP180_PRESSURE_ERROR_MMHG 6
//SDA A4
//SDL A5
#define DHTTYPE       DHT22 //DHT 22 (AM2302)

#define SYMBOLS_CELSIUM_GRAD ((byte)0x00)
#define SYMBOLS_MILLI_METER  ((byte)0x01)
#define SYMBOLS_HG           ((byte)0x02)
#define SYMBOLS_INVERT_MINUS ((byte)0x03)
#define SYMBOLS_INVERT_1     ((byte)0x04)
#define SYMBOLS_INVERT_2     ((byte)0x05)
#define SYMBOLS_INVERT_3     ((byte)0x06)
#define SYMBOLS_INVERT_NUM   ((byte)0x04)
#define SYMBOLS_FULL         ((byte)0xFF)

struct SHumiditySensor {
  float h;
  float t;
};

struct SPressureSensor {
  double p;
  double t;
};

struct SMeteoData {
  int time0;
  int time1;
  SHumiditySensor hs0;
  SHumiditySensor hs1;
  SPressureSensor ps;
};

// Custom symbols:
byte Symbols[][8] = {{  // SYMBOLS_CELSIUM_GRAD
    B11111,
    B10011,
    B10011,
    B11111,
    B11001,
    B11011,
    B11001,
    B11111
  }, {                  // SYMBOLS_MILLI_METER
    B10001,
    B11011,
    B10101,
    B00000,
    B10001,
    B11011,
    B10101,
    B00000
  }, {                  // SYMBOLS_HG
    B10100,
    B11100,
    B10100,
    B00011,
    B00100,
    B00101,
    B00011,
    B00000
  }, {                  // SYMBOLS_INVERT_MINUS
    B11111,
    B11111,
    B11111,
    B10001,
    B10001,
    B11111,
    B11111,
    B11111
  }, {                  // SYMBOLS_INVERT_1
    B11111,
    B11011,
    B10011,
    B11011,
    B11011,
    B11011,
    B10001,
    B11111
  }, {                  // SYMBOLS_INVERT_2
    B11111,
    B10001,
    B01110,
    B11101,
    B11011,
    B10111,
    B00000,
    B11111
  }, {                  // SYMBOLS_INVERT_3
    B11111,
    B00000,
    B11101,
    B11011,
    B11101,
    B01110,
    B10001,
    B11111
  }
};

// Global variables:
DHT dht0(DHT22_PIN0, DHTTYPE);
DHT dht1(DHT22_PIN1, DHTTYPE);
SFE_BMP180 bmp180;
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_DB4, LCD_DB5, LCD_DB6, LCD_DB7);
SMeteoData meteoData;

void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);
  while (!Serial) { ; } // wait for serial port to connect. Needed for native USB port only
  
  lcd.begin(16, 2);
  lcd.createChar(SYMBOLS_CELSIUM_GRAD,  Symbols[SYMBOLS_CELSIUM_GRAD]);
  lcd.createChar(SYMBOLS_MILLI_METER,   Symbols[SYMBOLS_MILLI_METER]);
  lcd.createChar(SYMBOLS_HG,            Symbols[SYMBOLS_HG]);
  lcd.createChar(SYMBOLS_INVERT_MINUS,  Symbols[SYMBOLS_INVERT_MINUS]);
  
  dht0.begin();
  dht1.begin();
  if (bmp180.begin()) {
    prn0("BMP180 init success");
  } else {
    prn0("BMP180 init fail");
  }

  meteoData.time0 = 0;
  meteoData.time1 = 0;
  delay(2000);
}

void loop() {
  getMeteoData();
  printSerialData();

// Prepare LCD
  lcd.clear();
  lcdPrn(0, 0, "  %     |00:00  ");
  lcdPrn(3, 0, SYMBOLS_FULL);
  lcdPrn(8, 0, SYMBOLS_CELSIUM_GRAD);
  lcdPrn(0, 1, "  %     |   h   ");
  lcdPrn(3, 1, SYMBOLS_FULL);
  lcdPrn(8, 1, SYMBOLS_CELSIUM_GRAD);
  lcdPrn(12, 1, SYMBOLS_HG);

// Print data
  lcdPrn(0, 0, (int)round(meteoData.hs0.h));
  lcdPrn(0, 1, (int)round(meteoData.hs1.h));

  lcdPrnT(0, (meteoData.hs0.t + meteoData.ps.t) / 2);
  lcdPrnT(1, meteoData.hs1.t);

  lcdPrnTm(0, meteoData.time0);
  lcdPrnTm(1, meteoData.time1);
  lcdPrn(9, 1, (int)round(mb2mmhg(meteoData.ps.p)));

  delay(5000);  // Pause for 5 seconds.

  if (Serial.available() > 0) {
    meteoData.time0 = Serial.parseInt();
    meteoData.time1 = Serial.parseInt();
  }
}

void getMeteoData() {
  meteoData.hs0.h = dht0.readHumidity();
  meteoData.hs0.t = dht0.readTemperature();
  meteoData.hs1.h = dht1.readHumidity();
  meteoData.hs1.t = dht1.readTemperature();
  char status = bmp180.startTemperature();
  if (status != 0) {
    // Wait for the measurement to complete:
    delay(status);
    status = bmp180.getTemperature(meteoData.ps.t);
    if (status != 0) {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.
      status = bmp180.startPressure(3);
      if (status != 0) {
        // Wait for the measurement to complete:
        delay(status);
        bmp180.getPressure(meteoData.ps.p, meteoData.ps.t);
      } else prn0("error starting pressure");
    } else prn0("error retrieving temperature");
  } else prn0("error starting temperature");
}

void printSerialData() {
  Serial.print(" ");
  Serial.print(meteoData.hs0.h, 1);
  Serial.print(" ");
  Serial.print(meteoData.hs0.t, 1);
  Serial.print(" ");
  Serial.print(meteoData.hs1.h, 1);
  Serial.print(" ");
  Serial.print(meteoData.hs1.t, 1);
  Serial.print(" ");
  Serial.print(meteoData.ps.p, 1);
  Serial.print(" ");
  Serial.print(meteoData.ps.t, 1);
  Serial.println("");
}

void prn0(const char* s) {
  Serial.println(s);
  lcd.setCursor(0, 0);
  lcd.print(s);  
}

void lcdPrnT(int y, float t) {
  if (t < 0) {
    t = -t;
    lcdPrn(3, y, SYMBOLS_INVERT_MINUS);
  } else {
    lcdPrn(3, y, SYMBOLS_FULL);
  }
  if (t >= 10) {
    lcdPrn(4, y, t, 1);
  } else {
    lcdPrn(5, y, t, 1);
  }
}

void lcdPrnTm(int y, int t) {
  if (t < 9) {
    lcdPrn(15, y, t);
  } else if (t < 19) {
    lcd.createChar(SYMBOLS_INVERT_NUM + y,      Symbols[SYMBOLS_INVERT_1]);
    lcdPrn(14, y, (byte)(SYMBOLS_INVERT_NUM + y));
    lcdPrn(15, y, (int)(t % 10));
  } else if (t < 29) {
    lcd.createChar(SYMBOLS_INVERT_NUM + y,      Symbols[SYMBOLS_INVERT_2]);
    lcdPrn(14, y, (byte)(SYMBOLS_INVERT_NUM + y));
    lcdPrn(15, y, (int)(t % 20));
  } else {
    lcd.createChar(SYMBOLS_INVERT_NUM + y,      Symbols[SYMBOLS_INVERT_3]);
    lcdPrn(14, y, (byte)(SYMBOLS_INVERT_NUM + y));
    lcdPrn(15, y, (int)(t % 30));
  }
}

void lcdPrn(int x, int y, const byte c) {
  lcd.setCursor(x, y);
  lcd.write(c);
}

void lcdPrn(int x, int y, const char* c) {
  lcd.setCursor(x, y);
  lcd.print(c);
}

void lcdPrn(int x, int y, const double d, const int format) {
  lcd.setCursor(x, y);
  lcd.print(d, format);
}

void lcdPrn(int x, int y, const int i) {
  lcd.setCursor(x, y);
  lcd.print(i);
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
