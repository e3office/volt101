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

		static constexpr char HTTPREQ_PREFIX[]="GET /";
		static constexpr size_t LENGTH_PREFIX=sizeof(HTTPREQ_PREFIX)-1;
		char cBufferPrefix[LENGTH_PREFIX+1]={0};

		static constexpr size_t LENGTH_LOCATION=12; // "/YYMMDD.dat "
		char cBufferLocation[LENGTH_LOCATION+1]="/";
		int iBufferLocationIndex=1;

		static constexpr char HTTPREQ_SUFFIX[]="\r\n\r\n";
		static constexpr size_t LENGTH_SUFFIX=sizeof(HTTPREQ_SUFFIX)-1;
		char cBufferSuffix[LENGTH_SUFFIX+1]={0};

		enum class ParseStat
		{
			WaitPrefix,
			CaptureLoc,
			WaitSuffix
		};
		ParseStat xParseStat=ParseStat::WaitPrefix;

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
						switch(xParseStat)
						{
							case ParseStat::WaitPrefix:
							{
								for(i=0;i<LENGTH_PREFIX-1;i++)
								{
									cBufferPrefix[i]=cBufferPrefix[i+1];
								}
								cBufferPrefix[LENGTH_PREFIX-1]=xClient.read();

								if(strcmp(cBufferPrefix,HTTPREQ_PREFIX)==0)
								{
									xParseStat=ParseStat::CaptureLoc;
								}
							} break;

							case ParseStat::CaptureLoc:
							{
								if(iBufferLocationIndex<LENGTH_LOCATION)
								{
									cBufferLocation[iBufferLocationIndex]=xClient.read();
									if(cBufferLocation[iBufferLocationIndex]==' ')
									{
										cBufferLocation[iBufferLocationIndex]='\0';
									}
									iBufferLocationIndex++;
								}
								else
								{
									xParseStat=ParseStat::WaitSuffix;
								}
							} break;

							case ParseStat::WaitSuffix:
							{
								for(i=0;i<LENGTH_SUFFIX-1;i++)
								{
									cBufferSuffix[i]=cBufferSuffix[i+1];
								}
								cBufferSuffix[LENGTH_SUFFIX-1]=xClient.read();

								if(strcmp(cBufferSuffix,HTTPREQ_SUFFIX)==0)
								{
									xClient.println("HTTP/1.1 200 OK");
									xClient.println("Content-Type: text/html");
									xClient.println("Connection: close");
									xClient.println();
									xClient.println("<!DOCTYPE html><html><body><h1>It works!</h1></body></html>");
									xClient.stop();
								}
							} break;

							default: break;
						}
					}
					else vTaskDelay(pdMS_TO_TICKS(1));
				}
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
