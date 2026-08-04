#include <Arduino.h>
#include <M5Unified.h>

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
		uint32_t ulPage=0UL;
		unsigned short usIndex=0U;
		m5::rtc_datetime_t xLastDateTime{};

		for(;;)
		{
			if(xTaskNotifyWait(0UL,0xffffffffUL,&ulPage,portMAX_DELAY)==pdPASS)
			{
				bBusy=true;

				// ...

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
