# Project
C++ Project to use Dual Core of the ESP32 to control led with display pattern and handle command from a Telegram Bot thanks to tasks

First Core handle wifi with a telegram API interface over a task and the Second Core handle the FastLED API with 60fps and task delay (allow a delay without freezing the whole esp32)

# Librairies

- Arduino
- C++
- UniversalTelegramBot
- FastLED