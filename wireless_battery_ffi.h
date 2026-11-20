#pragma once
#include <windows.h>

extern "C" {
char* wb_poll_devices_json();
void wb_free_string(char* ptr);
}