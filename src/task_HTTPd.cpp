#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <SD.h>
#include <string.h>
#include <ctype.h>

#include <freertos/semphr.h>

#include "common.h"
#include "helper.h"

namespace task_HTTPd
{
	static TaskHandle_t xTaskHandle=nullptr;

	constexpr uint16_t PORT_HTTP=80U;
	static WiFiServer xHttpServer(PORT_HTTP);

	enum class HttpResponse
	{
		Error400_badRequest,
		Error404_notFound,
		Error408_requestTimeOut,
		Error503_serviceUnavailable,
		Error500_internalServerError
	};

	static const char *httpErrorString(HttpResponse xError)
	{
		switch(xError)
		{
			case HttpResponse::Error400_badRequest:            return("400 Bad Request");           break;
			case HttpResponse::Error404_notFound:              return("404 Not Found");             break;
			case HttpResponse::Error408_requestTimeOut:        return("408 Request Timeout");       break;
			case HttpResponse::Error503_serviceUnavailable:    return("503 Service Unavailable");   break;

			//case HttpResponse::Error500_internalServerError:
			default:                                           return("500 Internal Server Error"); break;
		}
	}

	static void sendErrorResponse(WiFiClient& xClient,HttpResponse xError)
	{
		xClient.printf("HTTP/1.1 %s\r\n",httpErrorString(xError));
		xClient.println("Content-Type: text/plain");
		xClient.println("Connection: close");
		xClient.println();
		xClient.println(httpErrorString(xError));
	}

	static void handleResponse(WiFiClient& xClient,const char *pcLocation)
	{
		constexpr uint32_t MUTEX_BLOCKTIME_MS=3000UL;

		File xFile_rootDir;
		File xFile_nextFile;

		constexpr size_t LENGTH_BUFFER=64;
		unsigned char aucBuffer[LENGTH_BUFFER];
		int iBytesToRead;
		int iBytesWasRead;

		const char *pcFilename;

		if(common::xMutexMemCard==nullptr ||
		   xSemaphoreTake(common::xMutexMemCard,pdMS_TO_TICKS(MUTEX_BLOCKTIME_MS))!=pdTRUE)
		{
			sendErrorResponse(xClient,HttpResponse::Error503_serviceUnavailable);
			return;
		}

		if(pcLocation[1])
		{
			if(!helper::isValidFilename(&pcLocation[1]))
			{
				sendErrorResponse(xClient,HttpResponse::Error400_badRequest);
			}
			else if(!(xFile_nextFile=SD.open(pcLocation,FILE_READ)))
			{
				sendErrorResponse(xClient,HttpResponse::Error404_notFound);
			}
			else
			{
				xClient.println("HTTP/1.1 200 OK");
				xClient.println("Content-Type: text/csv");
				xClient.println("Connection: close");
				xClient.println();

				while(iBytesToRead=xFile_nextFile.available())
				{
					if(iBytesToRead>LENGTH_BUFFER) iBytesToRead=LENGTH_BUFFER;

					iBytesWasRead=xFile_nextFile.read(aucBuffer,iBytesToRead);
					if(iBytesWasRead<=0) break;

					xClient.write(aucBuffer,iBytesWasRead);
					vTaskDelay(pdMS_TO_TICKS(1));
				}
				xFile_nextFile.close();
			}
		}
		else
		{
			if(!(xFile_rootDir=SD.open("/")))
			{
				sendErrorResponse(xClient,HttpResponse::Error500_internalServerError);
			}
			else
			{
				xClient.println("HTTP/1.1 200 OK");
				xClient.println("Content-Type: text/html");
				xClient.println("Connection: close");
				xClient.println();
				xClient.println("<!DOCTYPE html><html><head><style type=\"text/css\">");
				xClient.println(".file-container {display:flex; flex-wrap:wrap; gap:15px; padding:10px;}");
				xClient.println(".file-item {display:block; padding:12px 20px; background:#f0f0f0; text-decoration:none; color:#333; border-radius:5px;}");
				xClient.println("</style></head><body><div class=\"file-container\">");

				for(;;)
				{
					if(!(xFile_nextFile=xFile_rootDir.openNextFile())) break;
					if(!xFile_nextFile.isDirectory())
					{
						pcFilename=xFile_nextFile.name();
						if(helper::isValidFilename(pcFilename))
						{
							xClient.printf("<a class=\"file-item\" href=\"/%1$s\">%1$s</a>\r\n",pcFilename);
						}
					}
					xFile_nextFile.close();
				}
				xFile_rootDir.close();

				xClient.println("</div></body></html>");
			}
		}

		xSemaphoreGive(common::xMutexMemCard);
	}

	static void vTaskMain(void *pvParameters)
	{
		int i;
		WiFiClient xClient;

		static constexpr unsigned long MILLIS_TIMEOUT=2000UL;
		unsigned long ulMills_connected;

		static constexpr char HTTPREQ_PREFIX[]="GET /";
		static constexpr size_t LENGTH_PREFIX=sizeof(HTTPREQ_PREFIX)-1;
		char acBufferPrefix[LENGTH_PREFIX+1]={0};

		static constexpr size_t LENGTH_LOCATION=12; // "/YYMMDD.dat "
		char acBufferLocation[LENGTH_LOCATION+1]="/";
		int iBufferLocationIndex;

		static constexpr char HTTPREQ_SUFFIX[]="\r\n\r\n";
		static constexpr size_t LENGTH_SUFFIX=sizeof(HTTPREQ_SUFFIX)-1;
		char acBufferSuffix[LENGTH_SUFFIX+1]={0};

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
				xParseStat=ParseStat::WaitPrefix;

				while(xClient.connected())
				{
					if(millis()-ulMills_connected>=MILLIS_TIMEOUT)
					{
						sendErrorResponse(xClient,HttpResponse::Error408_requestTimeOut);
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
									acBufferPrefix[i]=acBufferPrefix[i+1];
								}
								acBufferPrefix[LENGTH_PREFIX-1]=xClient.read();

								if(strcmp(acBufferPrefix,HTTPREQ_PREFIX)==0)
								{
									iBufferLocationIndex=1;
									xParseStat=ParseStat::CaptureLoc;
								}
							} break;

							case ParseStat::CaptureLoc:
							{
								if(iBufferLocationIndex<LENGTH_LOCATION)
								{
									acBufferLocation[iBufferLocationIndex]=xClient.read();
									if(acBufferLocation[iBufferLocationIndex]==' ')
									{
										acBufferLocation[iBufferLocationIndex]='\0';
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
									acBufferSuffix[i]=acBufferSuffix[i+1];
								}
								acBufferSuffix[LENGTH_SUFFIX-1]=xClient.read();

								if(strcmp(acBufferSuffix,HTTPREQ_SUFFIX)==0)
								{
									handleResponse(xClient,acBufferLocation);
									/*xClient.println("HTTP/1.1 200 OK");
									xClient.println("Content-Type: text/html");
									xClient.println("Connection: close");
									xClient.println();
									xClient.printf("<!DOCTYPE html><html><body><h1>It works!</h1>You requested '%s' (%d).\r\n",acBufferLocation,iBufferLocationIndex);
									for(i=0;i<=LENGTH_LOCATION;i++) xClient.printf("(%02x)",acBufferLocation[i]);
									xClient.println("</body></html>");*/
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
