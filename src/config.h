#pragma once

#include <string>

using namespace std;

// WiFi credentials
const String SSID = "Andared";
const String PASS = "llevalatararaunvestidoblancollenodecascabeles";

const string ID = "aa6ba08d-0ca2-41ec-8d4c-9e56112ff81f";
#define LOCAL_BROKER
#ifdef LOCAL_BROKER
const string BROKER = "192.168.13.169";
#else
const string BROKER = "test.mosquitto.org";
#endif
const string CLIENT_NAME = ID + "nightlight_client";
const string CLIENT_TELEMETRY_TOPIC = ID + "/telemetry";
const string TIME_SERVER = "es.pool.ntp.org";
