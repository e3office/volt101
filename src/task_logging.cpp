#include <Arduino.h>
#include <M5Unified.h>
#include <SD.h>

#include "common.h"
#include "task_adc.h"

namespace task_logging
{
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
			xTaskCreatePinnedToCore(vTaskMain,"Task_Logging",4096,NULL,2,&xTaskHandle,1);
		}
	}
}
