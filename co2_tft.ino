// A simple CO2 meter using the Adafruit SCD30 breakout
// using example for esp32s2 tft in adafruit ST7789 library
#include <Adafruit_SCD30.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "esp_wpa2.h" //wpa2 library for connections to Enterprise networks

// google docs certificate
const char* root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
    "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
    "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n" \
    "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n" \
    "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n" \
    "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n" \
    "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n" \
    "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n" \
    "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n" \
    "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n" \
    "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n" \
    "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n" \
    "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n" \
    "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n" \
    "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n" \
    "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n" \
    "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n" \
    "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n" \
    "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n"                            \
    "-----END CERTIFICATE-----\n";

WiFiClientSecure client;

#include "private.h"
#define EAP_IDENTITY "anonymous@wisc.edu"
// #define EAP_USERNAME "username" // defined in private.h
// #define EAP_PASSWORD "password" //your Eduroam password
const char* ssid = "eduroam"; // Eduroam SSID

const char* api_host = "docs.google.com";

bool wifi_connected=1;
#define MAX_WIFI_TRIES 10
#define MS_BETW_POSTS 60000
#define MS_TIMEOUT 5000
unsigned long last_post_time=0;

Adafruit_SCD30  scd30;
// Use dedicated hardware SPI pins
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

String sessionID;

void setup(void) {
  // Try to initialize!
  while (!scd30.begin()) {
    delay(250);
  }

  WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
  WiFi.mode(WIFI_STA); //init wifi mode

  // Example1 (most common): a cert-file-free eduroam with PEAP (or TTLS)
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);

  int wifi_tries = 0;
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    wifi_tries++;
    if(wifi_tries > MAX_WIFI_TRIES) {
      wifi_connected=0;
      break;
    }
  }

  client.setCACert(root_ca);

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

  // create a random session ID (15 random digits)
  randomSeed(analogRead(0));
  for(int i=0; i<15; i++) {
    sessionID += random(0,9);
  }

  // wait 10 seconds for the sensor to settle
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(3);
  tft.setCursor(0, 30);
  tft.println("Warming up...");
  delay(10000);
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

      const int httpPort = 443;
      if(!client.connect(api_host, httpPort)) {
          return;
      }

      // build up URI for request
      String url = "https://";
      url += api_host;
      url += PRIVATE_API_CALL; // defined in private.h
      url += "&";
      url += PRIVATE_ENTRY1; // defined in private.h
      url += "=";
      url += SERIAL_NUMBER;  // defined in private.h
      url += "&";
      url += PRIVATE_ENTRY2; // defined in private.h
      url += "=";
      url += sessionID; // random
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

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + api_host + "\r\n" +
                   "Connection: close\r\n\r\n");
      unsigned long timeout = millis();
      while(client.available() == 0) {
          if(millis() - timeout > MS_TIMEOUT) {
              client.stop();
              return;
          }
      }

      client.stop();
    } // end post form data

  delay(100);
}
