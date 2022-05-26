#ifndef _WIFI_SERVER_HANDLER_H_
#define _WIFI_SERVER_HANDLER_H_

#include <WiFi.h>
#include <WiFiClient.h>
#include "nvs_flash.h"

#define SSID_LENGTH 40
#define HOME_HTML "<!DOCTYPE html><html><head><title>WIFI SETTER</title><meta charset='UTF-8'/><style>body{background-color: #000000;font-family: \"Lucida Console\", \"Courier New\", monospace;}h1, div{margin-left: auto;margin-right: auto;width: 70%;}input,label{padding: 2% 2% 2% 2%;margin: 8px 0;color: #e6e6e6;}input{border: 1px solid #e6e6e6;border-radius: 4px;width: 50%;}label{width: 30%;display: inline-block;}div,input{background-color: #121212;}</style></head><body><h1 style=\"color:#e6e6e6;text-align: center;\">GMX LED LAMP WIFI SETTER</h1><div><form action=\"/set_over\"><div><label>SSID:</label><input type=\"text\" name=\"ssid\" placeholder=\"WIFI NETWORK NAME\"><br><label>PASSWORD:</label><input type=\"text\" name=\"password\" placeholder=\"WIFI PASSWORD\"></div><input style=\"width:100%;background-color:#2d3a9f;border:none\" type=\"submit\" value=\"CONNECT\"></form></div></body></html>"
#define SUCCESS_HTML "<!DOCTYPE html><html><head><title>WIFI SETTER</title><meta charset='UTF-8'/><style>body{background-color: #000000;font-family: \"Lucida Console\", \"Courier New\", monospace;}h1,h2{margin-left: auto;margin-right: auto;width: 70%;padding: 2% 2% 2% 2%;margin: 8px 0;color: #e6e6e6;}</style></head><body><div><h1>GMX LED LAMP WIFI SETTER</h1></div><h2>Wifi connected</h2></body></html>"
#define FAIL_HTML "<!DOCTYPE html><html><head><title>WIFI SETTER</title><meta charset='UTF-8'/><style>*{background-color: #000000;font-family: \"Lucida Console\", \"Courier New\", monospace;color: #e6e6e6;text-align: center;margin: auto;margin: 8px 8px;}div{/* width: 70%; */padding: 2% 2% 2% 0%;background-color: #121212;}a,button{width: 100%;padding: 2% 0% 2% 0%;background-color: #2d3a9f;border: none}</style></head><body><h1>GMX LED LAMP WIFI SETTER</h1><div>Wifi connection failed<a href='/'><button>RETRY</button></a></div></body></html>"

void wifi_handler();

#endif
