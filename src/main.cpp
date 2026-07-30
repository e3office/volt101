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

	delay(100);
	disp_initial();

	if(!M5.Rtc.isEnabled())
	{
		error_rise(error::RiseType::RtcUnavailable);
		return;
	}
	if(M5.Rtc.getVoltLow()) // RTC lost the valid date & time
	{
		M5.Rtc.setDateTime(common::DATETIME_DUMMY);
	}

	if(!SD.begin(SD_CS))
	{
		error_rise(error::RiseType::MemoryCardFailure);
		return;
	}
	disp_symbolMemory(symbol::TwoState::OK);

	if(appConf_load())
	{
		task_network::initializeTask();
	}

	task_adc::initializeTask();
}

void loop()
{
	static int8_t iLastSeconds=-1;
	m5::rtc_datetime_t xCurrentDateTime;

	if(!error_handle())
	{
		if(task_network::bFatal)
		{
			error_rise(error::RiseType::WiFiConnectionFailure);
		}

		M5.Rtc.getDateTime(&xCurrentDateTime);
		if(iLastSeconds!=xCurrentDateTime.time.seconds)
		{
			disp_dateTime(xCurrentDateTime);

			update_statusWiFi();

			iLastSeconds=xCurrentDateTime.time.seconds;
		}
	}
	vTaskDelay(pdMS_TO_TICKS(100));
}
