
#ifndef WIFI_SERVER_H
#define WIFI_SERVER_H

#include <WiFi.h>
#include <ESPmDNS.h>

#include "ArduinoJson.h"

#include "common.h"
#include "config.h"
#include "cs_timing.h"
#include "sensors.h"
#include "thermal_control.h"
#include "motion_control.h"

// Set web server port number to 80
extern WiFiServer server;
extern const char* ssid;
extern const char* password;
extern String header;

void begin_server();
void print_thermal_status_wifi();
void wifi_status_task(void * parameter);
void start_mdns_service();




#endif
