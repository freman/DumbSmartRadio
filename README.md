# Dumb Smart Radio

I needed a radio receiver that would just turn on when I applied power and remember what station/volume it was at... Surprisingly few affordable retail options.

## Requirements

* ESP8266 - This was built around a WEMO D1 Mini
* RDA5807 - The actual FM receiever, I will one day provide schematics for both starting from scratch, and starting from those tiny modules.

## Features

Super simple API

* /setVolume?volume=10 - Set the volume to 10
* /setFrequency?frequency=8980 - Set the frequency to 89.8
* /setBass?bass=1 - Turn on bass boost
* /save - Save values to EEPROM
