#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>

#include "common.h"
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

		static unsigned short usTimerSeconds=common::SECONDS_TO_SHUTDOWN;
		static unsigned char ucPower=0U;

		ucPower<<=1;
		if(M5.Power.Axp192.readRegister8(ADDR_AXP192_INPOWSTAT) & VAL_AXP192_INPOWSTAT_ACIN_AVAIL)
		{
			ucPower|=0x01U;
		}

		if(ucPower)
		{
			disp_symbolPower(symbol::TwoState::OK);
			usTimerSeconds=common::SECONDS_TO_SHUTDOWN;
			return(true);
		}
		else
		{
			disp_symbolPower(symbol::TwoState::NG);
			if(M5.Power.getBatteryLevel()<=common::BATTLEVEL_TO_SHUTDOWN) usTimerSeconds=0U;
			if(usTimerSeconds==0U) return(false);
			usTimerSeconds--;
			return(true);
		}
	}

	bool isValidFilename(const char *pcFilename)
	{
		// YYMMDD.dat
		if(strlen(pcFilename)==10 &&
			isdigit(pcFilename[0]) &&
			isdigit(pcFilename[1]) &&
			isdigit(pcFilename[2]) &&
			isdigit(pcFilename[3]) &&
			isdigit(pcFilename[4]) &&
			isdigit(pcFilename[5]) &&
			pcFilename[6]=='.' &&
			pcFilename[7]=='d' &&
			pcFilename[8]=='a' &&
			pcFilename[9]=='t') return(true);

		else return(false);
	}
}
