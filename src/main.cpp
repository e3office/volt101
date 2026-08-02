/*
	AC Voltage Logger system firmware version 1.01

	2026.7 Enomoto Electronics Engineering

	Expected configurations:

		M5Stack / M5Stack Core2
			https://docs.m5stack.com/ja/core/core2

		ZMPT101B Voltage Transformer Module, AC Voltage Sensor
			https://www.amazon.co.jp/dp/B0DMDYXY8L

		M5Stack / Module Proto that is mounted with an input LPF and a stabilizing capacitor
			https://docs.m5stack.com/ja/module/proto

		M5Unified, Arduino / ESP-IDF Library for M5Stack Series Version 0.2.17
			https://github.com/m5stack/M5Unified

		M5GFX, Graphics library for M5Stack series Version 0.2.24
			https://github.com/m5stack/M5GFX

		SimpleFTPServer Library 3.0.2
			https://github.com/xreef/SimpleFTPServer

	Versions history:

		2026.7.XX version 1.01
*/

#include <Arduino.h>
#include <M5Unified.h>
#include <SD.h>

#include "appconf.h"
#include "common.h"
#include "disp.h"
#include "error.h"
#include "helper.h"
#include "task_adc.h"
#include "task_logging.h"
#include "task_network.h"

// Definitions and configurations for SPI accesses microSD memory
static constexpr uint8_t SD_CS=4U;

void setup()
{
	m5::M5Unified::config_t xM5UnifiedConfig;

	xM5UnifiedConfig=M5.config();
	xM5UnifiedConfig.internal_imu=false;
	xM5UnifiedConfig.internal_mic=false;
	xM5UnifiedConfig.internal_spk=false;
	M5.begin(xM5UnifiedConfig);

	vTaskDelay(pdMS_TO_TICKS(100));
	disp_initialize();

	if(!M5.Rtc.isEnabled())
	{
		error::rise(error::RiseType::RtcUnavailable);
		return;
	}
	if(M5.Rtc.getVoltLow()) // RTC lost the valid date & time
	{
		M5.Rtc.setDateTime(common::DATETIME_DUMMY);
	}

	if(!SD.begin(SD_CS))
	{
		error::rise(error::RiseType::MemoryCardFailure);
		return;
	}
	disp_symbolMemory(symbol::TwoState::OK);

	if(appConf_load())
	{
		task_network::initializeTask();
	}

	task_logging::initializeTask();
	task_adc::initializeTask();
	vTaskDelay(pdMS_TO_TICKS(1000));
}

void loop()
{
	static int8_t iLastSeconds=-1;
	m5::rtc_datetime_t xCurrentDateTime;

	struct task_adc::Result xResult;

	static bool bLogging=false;
	bool bIsDatetimeValid;
	bool bIsPowerOK;

	if(!error::handle())
	{
		if(task_network::bFatal)
		{
			error::rise(error::RiseType::WiFiConnectionFailure);
		}

		if(task_logging::bFatal)
		{
			error::rise(error::RiseType::MemoryCardFailure);
		}

		M5.Rtc.getDateTime(&xCurrentDateTime);
		if(iLastSeconds!=xCurrentDateTime.time.seconds)
		{
			iLastSeconds=xCurrentDateTime.time.seconds;

			task_adc::getResult(&xResult);
			if(xResult.ucVoltRMS<common::ADC_VOLT_VALID_FROM) xResult.ucVoltRMS=0U;
			disp_currentResult(xResult);

			helper::updateStatusWiFi();
			bIsPowerOK=helper::powerOK();
			bIsDatetimeValid=disp_dateTime(xCurrentDateTime);

			if(bIsPowerOK)
			{
				if(bIsDatetimeValid)
				{
					if(!bLogging)
					{
						bLogging=true;
						disp_symbolLogger(symbol::StopRec::Rec);
					}
					task_logging::pushResult(xResult,xCurrentDateTime);
				}
			}
			else error::shutdownSafely();
		}
	}
	vTaskDelay(pdMS_TO_TICKS(100));
}
