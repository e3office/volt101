#include <Arduino.h>
#include <M5Unified.h>
#include <SD.h>
#include <string.h>

#include <freertos/semphr.h>

#include "common.h"
#include "helper.h"
#include "task_adc.h"

namespace task_logging
{
	constexpr unsigned long long HOUSEKEEPING_BYTES=32ULL*1024ULL*1024ULL;

	volatile bool bBusy=false;
	volatile bool bFatal=false;

	struct LogRecord
	{
		unsigned char ucVoltRMS;
		m5::rtc_datetime_t xDateTime;
	};
	constexpr unsigned char PAGES_LOGBUFFER=2U;
	constexpr unsigned short SIZE_LOGBUFFER=300U;
	static struct LogRecord axLogBuffer[PAGES_LOGBUFFER][SIZE_LOGBUFFER];

	static TaskHandle_t xTaskHandle=nullptr;

	void pushResult(const struct task_adc::Result& xResult,
	                const m5::rtc_datetime_t& xDateTime)
	{
		static unsigned char ucPage=0U;
		static unsigned short usIndex=0U;

		axLogBuffer[ucPage][usIndex].ucVoltRMS=xResult.ucVoltRMS;
		axLogBuffer[ucPage][usIndex].xDateTime=xDateTime;

		if(++usIndex>=SIZE_LOGBUFFER)
		{
			xTaskNotify(xTaskHandle,ucPage,eSetValueWithOverwrite);
			ucPage=(ucPage? 0U: 1U);
			usIndex=0U;
		}
	}

	static void doHousekeeping(void)
	{
		File xFile_rootDir;
		File xFile_nextFile;

		char acFilename_oldest[]="/999999.dat";
		const char *pcFilename_current;

		bool bFound=false;

		if((SD.totalBytes()-SD.usedBytes())>=HOUSEKEEPING_BYTES ||
		   !(xFile_rootDir=SD.open("/")))
		{
			return;
		}

		for(;;)
		{
			if(!(xFile_nextFile=xFile_rootDir.openNextFile())) break;
			if(!xFile_nextFile.isDirectory())
			{
				pcFilename_current=xFile_nextFile.name();
				if(helper::isValidFilename(pcFilename_current))
				{
					if(strcmp(pcFilename_current,&acFilename_oldest[1])<0)
					{
						acFilename_oldest[1]=pcFilename_current[0];
						acFilename_oldest[2]=pcFilename_current[1];
						acFilename_oldest[3]=pcFilename_current[2];
						acFilename_oldest[4]=pcFilename_current[3];
						acFilename_oldest[5]=pcFilename_current[4];
						acFilename_oldest[6]=pcFilename_current[5];
						bFound=true;
					}
				}
			}
			xFile_nextFile.close();
		}
		xFile_rootDir.close();

		if(bFound) SD.remove(acFilename_oldest);
	}

	static void vTaskMain(void *pvParameters)
	{
		File xFile;
		uint32_t ulPage=0UL;
		unsigned short usIndex=0U;
		m5::rtc_datetime_t xLastDateTime;
		char acFileName[]="/YYMMDD.dat";

		for(;;)
		{
			if(xTaskNotifyWait(0UL,0xffffffffUL,&ulPage,portMAX_DELAY)==pdPASS)
			{
				bBusy=true;
				if(common::xMutexMemCard==nullptr ||
				   xSemaphoreTake(common::xMutexMemCard,portMAX_DELAY)!=pdTRUE)
				{
					bFatal=true;
					bBusy=false;
					continue;
				}

				doHousekeeping();

				xLastDateTime=common::DATETIME_DUMMY;
				for(usIndex=0U;usIndex<SIZE_LOGBUFFER;usIndex++)
				{
					if(xLastDateTime.date.year !=axLogBuffer[ulPage][usIndex].xDateTime.date.year ||
					   xLastDateTime.date.month!=axLogBuffer[ulPage][usIndex].xDateTime.date.month ||
					   xLastDateTime.date.date !=axLogBuffer[ulPage][usIndex].xDateTime.date.date)
					{
						xLastDateTime=axLogBuffer[ulPage][usIndex].xDateTime;

						acFileName[1]='0'+axLogBuffer[ulPage][usIndex].xDateTime.date.year%100/10;
						acFileName[2]='0'+axLogBuffer[ulPage][usIndex].xDateTime.date.year%10;
						acFileName[3]='0'+axLogBuffer[ulPage][usIndex].xDateTime.date.month%100/10;
						acFileName[4]='0'+axLogBuffer[ulPage][usIndex].xDateTime.date.month%10;
						acFileName[5]='0'+axLogBuffer[ulPage][usIndex].xDateTime.date.date%100/10;
						acFileName[6]='0'+axLogBuffer[ulPage][usIndex].xDateTime.date.date%10;

						if(xFile) xFile.close();
						if(!(xFile=SD.open(acFileName,FILE_APPEND)))
						{
							bFatal=true;
							break;
						}
					}
					if(xFile.printf("%d,%d,%d,%u\r\n",
						axLogBuffer[ulPage][usIndex].xDateTime.time.hours,
						axLogBuffer[ulPage][usIndex].xDateTime.time.minutes,
						axLogBuffer[ulPage][usIndex].xDateTime.time.seconds,
						axLogBuffer[ulPage][usIndex].ucVoltRMS)<=0)
					{
						bFatal=true;
						break;
					}
				}

				if(xFile) xFile.close();
				xSemaphoreGive(common::xMutexMemCard);
				bBusy=false;
			}
		}
	}

	void initializeTask(void)
	{
		if(xTaskHandle==nullptr)
		{
			xTaskCreatePinnedToCore(
				vTaskMain,
				"Task_Logging",
				common::TASKCONF_LOGGING.ulStackSize,
				NULL,
				common::TASKCONF_LOGGING.uxPriority,
				&xTaskHandle,
				common::TASKCONF_LOGGING.xCoreID);
		}
	}
}
