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

There is a `private.h` file that is not included in the repository
that defines the serial number of the device (where it will be
convenient to change), the wifi SSID and password, the API call with
the google form identifier, and then the cumbersome form element IDs.

```
#define SERIAL_NUMBER "SOME S/N STRING"
#define PRIVATE_SSID "wifi_ssid"
#define PRIVATE_PASSWORD "wifi_password"

#define PRIVATE_API_CALL "/forms/d/e/[long form identifier]/formResponse?submit=Submit&usp=pp_url"

#define PRIVATE_ENTRY1 "entry.1234567"
#define PRIVATE_ENTRY2 "entry.2345678"
#define PRIVATE_ENTRY3 "entry.3456789"
#define PRIVATE_ENTRY4 "entry.4567890"
#define PRIVATE_ENTRY5 "entry.5678901"
```

---

### License

This work released under the [MIT License](LICENSE.md).
