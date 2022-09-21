// A simple CO2 meter using the Adafruit SCD30 breakout
// using example for esp32s2 tft in adafruit ST7789 library
#include <Adafruit_SCD30.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>


Adafruit_SCD30  scd30;
// Use dedicated hardware SPI pins
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup(void) {
  // Try to initialize!
  while (!scd30.begin()) {
    delay(250);
  }


  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  while (!scd30.setMeasurementInterval(2)){
      delay(250);
  }

  // initialize TFT
  tft.init(135, 240); // Init ST7789 240x135
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
}


void loop() {
  if (scd30.dataReady()) {
      tft.setTextWrap(false);
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(3);
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(0, 30);

    if (!scd30.read()){
      tft.println("READ ERR");
      return;
    }


    tft.print("CO2: ");

     if(scd30.CO2 > 1400) {
      tft.setTextColor(ST77XX_RED);

    } else if(scd30.CO2 > 1000) {
      tft.setTextColor(ST77XX_YELLOW);
    } else {
          tft.setTextColor(ST77XX_WHITE);
    }

    tft.print(scd30.CO2, 0);
    tft.println(" ppm");

    tft.setTextColor(ST77XX_WHITE);
    tft.print("Temp: ");
    tft.print(scd30.temperature*9.0/5.0+32.0, 0);
    tft.print("\367");
    tft.println("F");

    tft.print("Rel hum: ");
    tft.print(scd30.relative_humidity, 0);
    tft.println("%");

    }

  delay(100);
}
