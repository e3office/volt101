#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <string.h>

#include <freertos/semphr.h>

#include "common.h"

namespace task_HTTPd
{
	static TaskHandle_t xTaskHandle=nullptr;

	constexpr uint16_t PORT_HTTP=80U;
	static WiFiServer xHttpServer(PORT_HTTP);

	static void vTaskMain(void *pvParameters)
	{
		int i;
		WiFiClient xClient;

		static constexpr unsigned long MILLIS_TIMEOUT=2000UL;
		unsigned long ulMills_connected;

		static constexpr char TAIL_HTTPREQ[]="\r\n\r\n";
		static constexpr size_t LENGTH_TAIL_HTTPREQ=sizeof(TAIL_HTTPREQ)-1;
		char cBufferRx[LENGTH_TAIL_HTTPREQ+1]={0};

		xHttpServer.begin();

		for(;;)
		{
			xClient=xHttpServer.available();
			if(xClient)
			{
				ulMills_connected=millis();
				while(xClient.connected())
				{
					if(millis()-ulMills_connected>=MILLIS_TIMEOUT)
					{
						xClient.println("HTTP/1.1 408 Request Time-out");
						break;
					}

					if(xClient.available())
					{
						for(i=0;i<LENGTH_TAIL_HTTPREQ-1;i++)
						{
							cBufferRx[i]=cBufferRx[i+1];
						}
						cBufferRx[LENGTH_TAIL_HTTPREQ-1]=xClient.read();

						if(strcmp(cBufferRx,TAIL_HTTPREQ)==0)
						{
							xClient.println("HTTP/1.1 200 OK");
							xClient.println("Content-Type: text/html");
							xClient.println("Connection: close");
							xClient.println();
							xClient.println("<!DOCTYPE html><html><body><h1>It works!</h1></body></html>");
							break;
						}
					}
					else vTaskDelay(pdMS_TO_TICKS(1));
				}
				xClient.stop();
			}
			else vTaskDelay(pdMS_TO_TICKS(10));
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
