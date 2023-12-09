# Dumb Smart Radio

I needed a radio receiver that would just turn on when I applied power and remember what station/volume it was at... Surprisingly few affordable retail options.

## Requirements

* ESP8266 - This was built around a WEMO D1 Mini
* RDA5807 - The actual FM receiever, I will one day provide schematics for both starting from scratch, and starting from those tiny modules.

## Features

### Super simple API

* /set?args - Where ars can be any combination of the following seperated by &:
  * volume=10 - sets the volume to 10
  * frequency=8980 - sets the frequency to 89.8
  * bass=1 - enables bass boost
  * hostname=radio - sets a new host/mdns name
* /save - store current values persistently
* /reset - performs a soft reset on the tuner

#### Example request

http://radio.local/set?volume=5&bass=0
