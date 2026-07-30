#ifndef APPCONF_H
#define APPCONF_H

#include <stdint.h>

static constexpr const char *PATH_APPCONF="/SSID.ini";
static constexpr size_t LENGTH_FIELD_APPCONF=64;

struct AppConf
{
	char cWifiSsid[LENGTH_FIELD_APPCONF+1];
	char cWifiPass[LENGTH_FIELD_APPCONF+1];
	char cNtpServer[LENGTH_FIELD_APPCONF+1];
};

extern struct AppConf xAppConf;

bool appConf_load(void);

#endif // #ifndef APPCONF_H
