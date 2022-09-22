// A simple CO2 meter using the Adafruit SCD30 breakout
// using example for esp32s2 tft in adafruit ST7789 library
#include <Adafruit_SCD30.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <WiFi.h>

// private info
#include "private.h"
const char* ssid = PRIVATE_SSID;
const char* password = PRIVATE_PASSWORD;

const char* api_host = "docs.google.com";

bool wifi_connected=1;
#define MAX_WIFI_TRIES 10
#define MS_BETW_POSTS 60000
#define MS_TIMEOUT 5000
unsigned long last_post_time=0;


Adafruit_SCD30  scd30;
// Use dedicated hardware SPI pins
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup(void) {
  // Try to initialize!
  while (!scd30.begin()) {
    delay(250);
  }

  Serial.begin(115200);
  while(!Serial) { delay(100); }
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int wifi_tries = 0;
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    wifi_tries++;
    if(wifi_tries > MAX_WIFI_TRIES) {
      wifi_connected=0;
      break;
    }
  }

  if(wifi_connected) {
      Serial.println("Wifi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
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

    // post form data?
    if(wifi_connected && (last_post_time==0 || millis() - last_post_time > MS_BETW_POSTS)) {
      last_post_time = millis();

      WiFiClient client;
      const int httpPort = 80;
      if(!client.connect(api_host, httpPort)) {
          return;
      }

      // build up URI for request
      String url = PRIVATE_API_CALL; // defined in private.h
      url += "&";
      url += PRIVATE_ENTRY1; // defined in private.h
      url += "=";
      url += "ULC013";
      url += "&";
      url += PRIVATE_ENTRY2; // defined in private.h
      url += "=";
      url += "session001";
      url += "&";
      url += PRIVATE_ENTRY3; // defined in private.h
      url += "=";
      url += scd30.CO2;
      url += "&";
      url += PRIVATE_ENTRY4; // defined in private.h
      url += "=";
      url += scd30.temperature;
      url += "&";
      url += PRIVATE_ENTRY5; // defined in private.h
      url += "=";
      url += scd30.relative_humidity;

      Serial.println();
      Serial.println(url);

      client.print(String("Get ") + url + " HTTP/1.1\r\n" +
                   "Host: " + api_host + "\r\n" +
                   "Connection: close\r\n\r\n");
      unsigned long timeout = millis();
      while(client.available() == 0) {
          if(millis() - timeout > MS_TIMEOUT) {
              client.stop();
              return;
          }
      }


      Serial.println();
      while(client.available()) {
          String line = client.readStringUntil('\r');
          Serial.println(line);
      }
      Serial.println();

    } // end post form data

  delay(100);
}
