#include <Arduino.h>
#include <WiFi.h>

#include "disp.h"
#include "error.h"
#include "task_network.h"

namespace helper
{
	void updateStatusWiFi(void)
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

		if(task_network::bFatal) error::rise(error::RiseType::WiFiConnectionFailure);
	}

	bool powerOK/*updateStatusPower*/(void)
	{
		constexpr uint8_t ADDR_AXP192_INPOWSTAT=0U;
		constexpr uint8_t VAL_AXP192_INPOWSTAT_ACIN_AVAIL=0x40U;

		constexpr unsigned short SECONDS_TO_SHUTDOWN=600U;
		constexpr int32_t BATTLEVEL_TO_SHUTDOWN=50;

		static unsigned short usTimerSeconds=SECONDS_TO_SHUTDOWN;

		if(M5.Power.Axp192.readRegister8(ADDR_AXP192_INPOWSTAT) & VAL_AXP192_INPOWSTAT_ACIN_AVAIL)
		{
			disp_symbolPower(symbol::TwoState::OK);
			usTimerSeconds=SECONDS_TO_SHUTDOWN;
			return(true);
		}
		else
		{
			disp_symbolPower(symbol::TwoState::NG);
			if(M5.Power.getBatteryLevel()<=BATTLEVEL_TO_SHUTDOWN) usTimerSeconds=0U;
			if(usTimerSeconds==0U) return(false);
			usTimerSeconds--;
			return(true);
		}
	}
}
