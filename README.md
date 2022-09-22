## SCD-30 CO2 sensor with Adafruit ESP32-S3 TFT

Using the Adafruit ESP32-S3 TFT feather multicontroller with an SCD-30
CO2 sensor.

I'm most interested here in using the wifi chip to automatically log
data to google sheets (via a google form).

- Using the Adafruit library for the SCD-30
- Using the Adafruit library for the integrated TFT display
- Needed to download the SSL certificate for docs.google.com
- Can use a GET to post sensor data to the google form, which then
  shows up in a google sheet, but need to use SSL and so
  `WiFiClientSecure`
