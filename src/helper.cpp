#include <Arduino.h>
#include <WiFi.h>

#include "disp.h"
#include "error.h"
#include "task_network.h"

void update_statusWiFi(void)
{
	int32_t iRSSI;

	if(WiFi.status()!=WL_CONNECTED) disp_symbolWiFi(symbol::WiFi::NG);
	else
	{
		iRSSI=WiFi.RSSI();
		if(iRSSI>=-55)      disp_symbolWiFi(symbol::WiFi::Good4);
		else if(iRSSI>=-70) disp_symbolWiFi(symbol::WiFi::Good3);
		else if(iRSSI>=-85) disp_symbolWiFi(symbol::WiFi::Good2);
		else                disp_symbolWiFi(symbol::WiFi::Good1);
	}

	if(task_network::bFatal) error_rise(error::RiseType::WiFiConnectionFailure);
}
