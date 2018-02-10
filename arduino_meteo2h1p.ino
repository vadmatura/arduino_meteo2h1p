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
//SDA A4
//SDL A5

#define DHTTYPE     DHT22 //DHT 22 (AM2302)
DHT dht0(DHT22_PIN0, DHTTYPE);
DHT dht1(DHT22_PIN1, DHTTYPE);
SFE_BMP180 bmp180;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2); // (RS, E, DB4, DB5, DB6, DB7)

void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  lcd.begin(16, 2);
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

  Serial.print("0: ");
  Serial.print(humidity0, 1);
  Serial.print("% ");
  Serial.print(temperature0, 1);
  Serial.print("*C 1: ");
  Serial.print(humidity1, 1);
  Serial.print("% ");
  Serial.print(temperature1, 1);
  Serial.print("*C 2: ");
  Serial.print(pressure2, 1);
  Serial.print("mb ");
  Serial.print(temperature2, 1);
  Serial.println("*C");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("     *C   % mmHg");
  lcd.setCursor(5, 0);
  lcd.print((char)0xDF);
  lcd.setCursor(7, 0);
  lcd.print((char)0xFF);
  lcd.setCursor(11, 0);
  lcd.print((char)0xFF);
  lcd.setCursor(0, 1);
  lcd.print("     *C   %     ");
  lcd.setCursor(5, 1);
  lcd.print((char)0xDF);
  lcd.setCursor(7, 1);
  lcd.print((char)0xFF);
  lcd.setCursor(11, 1);
  lcd.print((char)0xFF);

  lcd.setCursor(0, 0);
  lcd.print((temperature0 + temperature2) / 2, 1);
  lcd.setCursor(0, 1);
  lcd.print(temperature1, 1);

  lcd.setCursor(8, 0);
  lcd.print((int)humidity0);
  lcd.setCursor(8, 1);
  lcd.print((int)humidity1);
  lcd.setCursor(13, 1);
  lcd.print((int)mb2mmhg(pressure2));

/*  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("____*C ____*C   ");
//           0123456789012345
  lcd.setCursor(0, 1);
  lcd.print("___% ___%  ___mm");
//           0123456789012345

  lcd.setCursor(0, 0);
  lcd.print(temperature0, 1);
  lcd.setCursor(7, 0);
  lcd.print(temperature1, 1);

  lcd.setCursor(0, 1);
  lcd.print((int)humidity0, 1);
  lcd.setCursor(5, 1);
  lcd.print((int)humidity1, 1);
  lcd.setCursor(11, 1);
  lcd.print((int)mb2mmhg(pressure2));*/

  delay(5000);  // Pause for 5 seconds.
}

void prn0(const char* s) {
  Serial.println(s);
  lcd.setCursor(0, 0);
  lcd.print(s);  
}

double mb2mmhg(double mb) {
  return mb * 0.750061683;
}

