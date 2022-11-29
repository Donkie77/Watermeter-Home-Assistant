# Watermeter-Home-Assistant-ESP8622

I was looking for a water meter sensor based on a proximity sensor for integration into Home Assistant. I found many solutions based on ESPHome which is great however I wanted to ensure my water consumption value was stored independant from Home Assistant. So in case of a failure of Home assistant, the sensor keeps on counting. Therefore I've created this code which was largly based on the repository of Jordan Crubin https://github.com/jordancrubin/watermeter but simplified, made working on my Wemos D1 mini and storing the meter value to the EEPROM of the ESP8266 (to ensure in case of power failure the meter value is retained).

Used parts:
- 1x Wemos D1 mini
- 1x USB cable + 230V/5V adapter (powering the D1)
- 1x M18 8mm DC5V NPN NO proximity sensor type LJ18A3-8-Z/BX-5V, connected directly to the 5V/GND of the D1 and one of the GPIO's.
- 1x Resistor 10K (as pull-up of the input)

The code contains some settings like WiFI, MQTT, GPIO pin, Pulse Factor and Start value.

In Home Assistant I've used the Mosquito MQTT broker and added the following MQTT sensors in my configuration.yaml:
```
### MQTT BROKER SENSORS ###
mqtt:
  sensor:
    - name: "Waterstand"
      state_topic: "WATERMETER/WATER/VOLUME"
      unit_of_measurement: "m³"
      icon: "mdi:water"
      force_update: true
      state_class: total_increasing
    - name: "Water Flow"
      state_topic: "WATERMETER/WATER/FLOW"
      unit_of_measurement: "L/min"
      icon: "mdi:water-sync"
      force_update: true
      state_class: measurement
      value_template: '{{value | round(1) }}'
 ```     
 And presented the 2 sensors in my dashboard.
 
 DISCLAIMER: I am an amature in coding so but since this is a  proper solution for my idea I am sharing this.
