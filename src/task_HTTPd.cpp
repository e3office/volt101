#include <Arduino.h>
#include <M5Unified.h>

#include "common.h"

namespace task_HTTPd
{
	static TaskHandle_t xTaskHandle=nullptr;

	static void vTaskMain(void *pvParameters)
	{
		for(;;)
		{
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}

	void disposeTask(void)
	{
		if(xTaskHandle!=nullptr)
		{
			vTaskDelete(xTaskHandle);
			xTaskHandle=nullptr;
		}
	}

	void initializeTask(void)
	{
		if(xTaskHandle==nullptr)
		{
			xTaskCreatePinnedToCore(
				vTaskMain,
				"Task_HTTPd",
				common::TASKCONF_HTTPD.ulStackSize,
				NULL,
				common::TASKCONF_HTTPD.uxPriority,
				&xTaskHandle,
				common::TASKCONF_HTTPD.xCoreID);
		}
	}
}
