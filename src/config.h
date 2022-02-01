#pragma once

#include <string>

using namespace std;

// WiFi credentials
char *SSID = "Andared";
char *PASSWORD = "llevalatararaunvestidoblancollenodecascabeles";

const string ID = "aa6ba08d-0ca2-41ec-8d4c-9e56112ff81f";

const string BROKER = "test.mosquitto.org";
const string CLIENT_NAME = ID + "nightlight_client";
const string CLIENT_TELEMETRY_TOPIC = ID + "/telemetry";
const string TIME_SERVER = "es.pool.ntp.org";
