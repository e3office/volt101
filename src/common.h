#ifndef COMMON_H
#define COMMON_H

#include <M5Unified.h>

#include <freertos/semphr.h>

struct TaskConf
{
	UBaseType_t uxPriority;
	BaseType_t xCoreID;
	uint32_t ulStackSize;
};

constexpr BaseType_t CORE0=0;
constexpr BaseType_t CORE1=1;

namespace common
{
	extern const m5::rtc_datetime_t DATETIME_DUMMY;
	constexpr int16_t YEAR_VALIDFROM=2026;

	constexpr int PIN_AI_VOLT=35;

	constexpr const char* LOCAL_TIMEZONE="JST-9";
	constexpr const char* LOCAL_HOSTNAME="volt";

	constexpr float NORMAL_VDD_MILLIVOLT=3300.0F;
	constexpr float NORMAL_ADC_OFFSET_MIN=NORMAL_VDD_MILLIVOLT*0.9F*0.4F;
	constexpr float NORMAL_ADC_OFFSET_MAX=NORMAL_VDD_MILLIVOLT*1.1F*0.6F;

	constexpr unsigned char ADC_VOLT_VALID_FROM=10U;

	constexpr unsigned short SECONDS_TO_SHUTDOWN=600U;
	constexpr int32_t BATTLEVEL_TO_SHUTDOWN=50;

	extern SemaphoreHandle_t xMutexMemCard;

	constexpr struct TaskConf TASKCONF_LOGGING={ 2,CORE1,4096U};
	constexpr struct TaskConf TASKCONF_ADC    ={21,CORE0,2048U};
	constexpr struct TaskConf TASKCONF_NETWORK={ 2,CORE0,4096U};
	constexpr struct TaskConf TASKCONF_HTTPD  ={ 1,CORE0,4096U};
}

#endif // #ifndef #COMMON_H
