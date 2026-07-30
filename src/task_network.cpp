#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <ESPmDNS.h>

// Different versions of the framework have different SNTP header file names and availability.
#if __has_include (<esp_sntp.h>)
	#include <esp_sntp.h>
	#define SNTP_ENABLED 1
#elif __has_include (<sntp.h>)
	#include <sntp.h>
	#define SNTP_ENABLED 1
#endif

#ifndef SNTP_ENABLED
	#error "SNTP is unavailable in this environment."
#endif

#include "appconf.h"
#include "common.h"
//#include "task_ftp.h"

namespace task_network
{
	volatile bool bFatal=false;

	//static bool bInProgressSNTP=false;

	enum class StateSyncSNTP : uint8_t
	{
		NotYet,
		InProgress,
		Completed,
		Failed
	};

	static StateSyncSNTP xStateSyncSNTP=StateSyncSNTP::NotYet;

	static void vSubTask_SNTP(void)
	{
		static constexpr unsigned POLLING_INTERVAL_MS=50U;
		static constexpr int POLLING_COUNT_MAX=40;

		time_t xLastTime,xCurrentTime;
		int i;

		if(xStateSyncSNTP==StateSyncSNTP::InProgress)
		{
			if(sntp_get_sync_status()==SNTP_SYNC_STATUS_COMPLETED)
			{
				time(&xLastTime);
				for(i=0;i<POLLING_COUNT_MAX;i++)
				{
					time(&xCurrentTime);
					if(xCurrentTime!=xLastTime)
					{
						M5.Rtc.setDateTime(localtime(&xCurrentTime));
						configTzTime(common::LOCAL_TIMEZONE,"");
						xStateSyncSNTP=StateSyncSNTP::Completed;
						return;
					}
					vTaskDelay(pdMS_TO_TICKS(POLLING_INTERVAL_MS));
				}
				configTzTime(common::LOCAL_TIMEZONE,"");
				xStateSyncSNTP=StateSyncSNTP::Failed;
			}
		}
	}

	static void vTaskMain(void *pvParameters)
	{
		wl_status_t xCurrentWiFiStatus;

		static wl_status_t xLastWiFiStatus=WL_CONNECTED;

		static int iCount_ConnectionAttempt=0;
		static constexpr int COUNT_CONNECTIONATTEMPT_LIMIT=20;

		for(;;)
		{
			xCurrentWiFiStatus=WiFi.status();

			if(xLastWiFiStatus==WL_CONNECTED && xCurrentWiFiStatus!=WL_CONNECTED)
			{
				if(++iCount_ConnectionAttempt>COUNT_CONNECTIONATTEMPT_LIMIT)
				{
					bFatal=true;
					vTaskDelete(NULL);
				}

				// stop something that depends on a network

				// SNTP client
				if(xStateSyncSNTP==StateSyncSNTP::InProgress)
				{
					configTzTime(common::LOCAL_TIMEZONE,"");
					xStateSyncSNTP=StateSyncSNTP::NotYet;
				}

				// mDNS responder
				MDNS.end();

				// FTP server
				//task_ftp::stopServer();

				WiFi.disconnect();
				vTaskDelay(pdMS_TO_TICKS(100));
				WiFi.begin(xAppConf.cWifiSsid,xAppConf.cWifiPass);
				vTaskDelay(pdMS_TO_TICKS(100));
			}
			else if(xLastWiFiStatus!=WL_CONNECTED && xCurrentWiFiStatus==WL_CONNECTED)
			{
				// start something that depends on a network

				// SNTP client
				if(xStateSyncSNTP==StateSyncSNTP::NotYet)
				{
					configTzTime(common::LOCAL_TIMEZONE,xAppConf.cNtpServer);
					xStateSyncSNTP=StateSyncSNTP::InProgress;
				}

				// mDNS responder
				MDNS.begin(common::LOCAL_HOSTNAME);

				// FTP server
				//task_ftp::startServer();
			}

			xLastWiFiStatus=xCurrentWiFiStatus;

			vSubTask_SNTP();

			vTaskDelay(pdMS_TO_TICKS(2000));
		}
	}

	void initializeTask(void)
	{
		WiFi.disconnect();
		WiFi.softAPdisconnect(true);
		WiFi.mode(WIFI_STA);

		xTaskCreatePinnedToCore(vTaskMain,"Task_Network",4096,NULL,1,NULL,0);
	}
}
