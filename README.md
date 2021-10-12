# AccessControl
ESP8266 based access control firmware to run on HSBNE's Sonoff TH10 based Doors and Interlocks.

# Hardware Types

There's two main box builds, door and interlock. (Pictures coming)

Door contains a Sonoff TH10, a 240v to 12v cage power supply and uses a GX16 4 pin connector for serial to an external RFID reader and a GX12 to the door bolt/strike.

Interlock contains a Sonoff TH10, a 240v 20A 2Pole NO contactor, a 3.3v RFID reader and a single 20mm threaded case WS2812b led on a custom PCB that is wired into a TRRS connector on the sonoff.

More details on the hardware to come, including pictures.

# Arduino Studio Setup
1. Download and install [Arduino Studio](https://www.arduino.cc/en/software)

2. In arduino studio, open library manager (CTRL+SHIFT+I).  
Search for and install the following libraries:
	* [WS2812FX (Version 1.3.4 by Harm Aldick)](https://github.com/kitesurfer1404/WS2812FX)
	* [Adafruit_NeoPixel (Version 1.10.0 by Adafruit)](https://github.com/adafruit/Adafruit_NeoPixel)
	* [ArduinoJson (Version 6.18.5 by Beniot Blachon)](https://arduinojson.org/?utm_source=meta&utm_medium=library.properties)
	* [Websockets (Version 2.3.5 by Marcus Sattler)](https://github.com/Links2004/arduinoWebSockets)  

3. Follow the ESP8266 'installing with boards manager' guide linked here:  
https://github.com/esp8266/Arduino#installing-with-boards-manager  
  
4. Set the board type to "Generic ESP8266 Module":  
Tools>Board: "xxxxx">ESP8266 Boards>Generic ESP8266 Module

5. Open one of the .ino files from this repository and press the 'Verify Button'. Ensure the sketch compiles successfully.