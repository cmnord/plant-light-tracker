# plant-light-tracker

Track how much light your plants receive each day! The system keeps track of the
total amount of time each day that the photocell records light above a certain
threshold.  The screen shows a friendly dashboard with today's sunlight and the
current brightness level.

Made for [MakeMIT][makemit] 2018.

# Hardware

- [Microcontroller][feather]: Adafruit Feather HUZZAH with ESP8266 WiFi
- [OLED][oled]: Diymall 0.96" yellow/blue I2C
- [Photocell][photocell] (light-dependent resistor)
- 10Kohm potentiometer
- 9.1Kohm resistor
- button

# Software

- [esp8266 core][esp8266] for Arduino (board config, ESP8266WiFi library)
- [u8g2][u8g2] graphics library
- Arduino [Time][time] library

# License

[MIT][license]

[makemit]: https://makemit.org/
[feather]: https://www.adafruit.com/product/2821
[oled]: http://www.diymalls.com/OLED/0.96-blue-and-yellow-oled-display
[photocell]: https://www.adafruit.com/product/161
[esp8266]: https://github.com/esp8266/Arduino
[u8g2]: https://github.com/olikraus/u8g2
[time]: https://github.com/PaulStoffregen/Time
[license]: https://clairenord.com/mitlicense
