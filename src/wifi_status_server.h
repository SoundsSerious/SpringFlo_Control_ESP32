
#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include <WiFi.h>
#include <ESPmDNS.h>

#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "common.h"
#include "config.h"
#include "cs_timing.h"
#include "sensors.h"
#include "thermal_control.h"
#include "motion_control.h"
#include "user_input.h"


// Set web server port number to 80
//extern WiFiServer server;

extern String header;

void begin_server();
void print_thermal_status_wifi();
void wifi_status_task(void * parameter);
void start_mdns_service();

void onRequest(AsyncWebServerRequest *request);

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

String add_field(String visible_name, String tag_name);
String add_input(String visible_name, String tag_name, int defaultvalue);
String html_doc();

#endif
