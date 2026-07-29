#ifndef COMMON_H
#define COMMON_H

#include <M5Unified.h>

namespace common
{
	extern const m5::rtc_datetime_t DATETIME_DUMMY;
	static constexpr int16_t YEAR_VALIDFROM=2026;

	static constexpr int PIN_AI_VOLT=35;

	static constexpr const char* LOCAL_TIMEZONE="JST-9";
	static constexpr const char* LOCAL_HOSTNAME="volt";
}

#endif // #COMMON_H DISP_H
