# Project

C++ Project to use Dual Core of the ESP32 to control led with display pattern and handle command from a Telegram Bot thanks to tasks

Create a WifiServer at `192.168.4.1` (check your esp ip addr) allowing users to enter their wifi credential and save it in ESP32 nvs.
While the Wifi doesn't change, the lamp auto-connects for a simpler use.
The lamp won't start since the Wifi isn't connected.

First Core handle wifi with a telegram API interface over a task and the Second Core handle the FastLED API with 60fps and some task delay (allow a delay without freezing the whole esp32 cores)

## Version

0.0.1: Telegram, Task and FastLed handling, one file.
0.0.2: Refacto with class and custom libraries, allow user to enter wifi credential.

# Librairies

- Arduino
- C++
- NVS
- FreeRTOS
- WifiServer - WifiSecureServer
- UniversalTelegramBot
- FastLED

# Installation

As Arduino doesn't detect directory in the main directory, all the `src` directories must be copied to your libraries folders.
While compiling, if the header files are missing, the libraries are misplaced.

# Circuit Notes

Followed this [best practices](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices) to mount the circuit: - 180 ws2812b neopixel leds, last 36 leds with power reinjection. Maybe, I will reinject power in the middle of the 144 leds. - Used a 220 Ohm resistor that I already have to connect esp32 PWM output to neopixel data input. - 3.3V to 5V logic level converter from sparkfun (amazon: YT1076) - About 60-80cm between the esp32 and the neopixel strips without signal loss.
