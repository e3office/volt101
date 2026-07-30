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

	static constexpr float NORMAL_VDD_MILLIVOLT=3300.0F;
	static constexpr float NORMAL_ADC_OFFSET_MIN=NORMAL_VDD_MILLIVOLT*0.9F*0.4F;
	static constexpr float NORMAL_ADC_OFFSET_MAX=NORMAL_VDD_MILLIVOLT*1.1F*0.6F;

	static constexpr unsigned char ADC_VOLT_VALID_FROM=10U;
}

#endif // #COMMON_H DISP_H
