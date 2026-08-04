#include <M5Unified.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "common.h"

namespace common
{
	const m5::rtc_datetime_t DATETIME_DUMMY={{2000,1,1},{0,0,0}};

	SemaphoreHandle_t xMutexMemCard=nullptr;
}
