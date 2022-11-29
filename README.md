# Watermeter-Home-Assistant

I was looking for a water meter sensor based on a proximity sensor for integration into Home Assistant. I found many solution based on ESPHome which is great however I wanted to ensure my water consumption value was stored independant from Home Assistant. So in case of a failure of Home assistant, the sensor keeps on counting. Therefore I've created this code which was highhly based on the repository of Jordan Crubin: https://github.com/jordancrubin/watermeter

- Counts pulses by a proximity sensor
- Calculates active flow liters/min.
- Stores the actual consumption value in the EEPROM of the ESP8266.
