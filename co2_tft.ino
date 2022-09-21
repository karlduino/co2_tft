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
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SCD30 CO2 meter!");

  // Try to initialize!
  if (!scd30.begin()) {
    Serial.println("Failed to find SCD30 chip");
    while (1) { delay(10); }
  }
  Serial.println("SCD30 Found!");


  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  if (!scd30.setMeasurementInterval(2)){
    Serial.println("Failed to set measurement interval");
    while(1) {delay(10);}
  }
  Serial.print("Measurement Interval: ");
  Serial.print(scd30.getMeasurementInterval());
  Serial.println(" seconds");

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
    
    Serial.println("Data available!");

    if (!scd30.read()){
      Serial.println("Error reading sensor data");
      tft.println("READ ERR");
      return;
    }

    Serial.print("CO2: ");
    Serial.print(scd30.CO2, 0);
    Serial.println(" ppm");
    Serial.println("");

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
  }

  delay(100);
}
