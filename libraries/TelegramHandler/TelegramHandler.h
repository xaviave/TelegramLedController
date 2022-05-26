#ifndef _TELEGRAM_HANDLER_H_
#define _TELEGRAM_HANDLER_H_

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Bot Token
#define CHAT_ID "XXXX"
#define BOT_TOKEN "XXXX"

// Telegram usage
#define WELCOME "Use the following commands to control your light.\nSend /on or /off to start\nSend /brightness [0-255] to brightness\nGet custom color with /color RGB HEX code\nGet preset color:\n/red | /light_red | /green | /lime | /blue | /cyan | /yellow | /orange | /purple | /pink | /magenta | /white | /warm_white\nGet different light mode:\n /solid_color | /random_solid | /random_gradient | /shine | /heartbeat | /rainbow | /moving_rainbow\nGet different rainbow mode:\n /rainbow_color | /rainbow_color_stripe | /rainbow_color_random"
#define ERROR_BRIGHTNESS "Error: Brightness must be between 0 and 255"

void handle_new_messages(QueueHandle_t g_xQueue);

#endif
