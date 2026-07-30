#ifndef DISP_H
#define DISP_H

#include <Arduino.h>
#include <M5Unified.h>

#include "error.h"
#include "task_adc.h"

namespace symbol
{
	enum class ThreeState : uint8_t {NG=0,OK,Unknown};
	enum class TwoState : uint8_t {NG=0,OK};
	enum class StopRec : uint8_t {Stop=0,Rec};
	enum class WiFi : uint8_t {NG=0,Good1,Good2,Good3,Good4};
}

void disp_symbolClock(symbol::ThreeState xSymbolThreeState);
void disp_symbolMemory(symbol::TwoState xSymbolTwoState);
void disp_symbolVolt(symbol::ThreeState xSymbolThreeState);
void disp_symbolLogger(symbol::StopRec xSymbolStopRec);
void disp_symbolWiFi(symbol::WiFi xSymbolWiFi);
void disp_symbolPower(symbol::TwoState xSymbolTwoState);

void disp_dateTime(const m5::rtc_datetime_t& xDateTime);
void disp_currentResult(const struct task_adc::Result& xResult);
void disp_initial(void);
void disp_error(error::RiseType xRiseType);

#endif // #ifndef DISP_H
