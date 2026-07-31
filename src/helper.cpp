#include <Arduino.h>
#include <WiFi.h>

#include "disp.h"
#include "error.h"
#include "task_network.h"

void helper_updateStatusWiFi(void)
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

void helper_updateStatusPower(void)
{
	constexpr uint8_t ADDR_AXP192_INPOWSTAT=0U;
	constexpr uint8_t VAL_AXP192_INPOWSTAT_ACIN_AVAIL=0x40U;

	if(M5.Power.Axp192.readRegister8(ADDR_AXP192_INPOWSTAT) & VAL_AXP192_INPOWSTAT_ACIN_AVAIL)
	{
		disp_symbolPower(symbol::TwoState::OK);
	}
	else disp_symbolPower(symbol::TwoState::NG);
}
