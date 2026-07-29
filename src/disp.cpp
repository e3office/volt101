#include <Arduino.h>
#include <M5Unified.h>

#include "bitmaps.h"
#include "common.h"
#include "disp.h"
#include "error.h"

static void disp_graphFrame(LovyanGFX& xDispTo,int32_t iOffsetX,int32_t iOffsetY)
{
	xDispTo.drawRect(  0+iOffsetX, 16+iOffsetY,278,100,DARKGREY);
}

static void disp_graphScale(LovyanGFX& xDispTo,int32_t iOffsetX,int32_t iOffsetY)
{
	xDispTo.drawLine( 37+iOffsetX, 16+iOffsetY, 37+iOffsetX,115+iOffsetY,DARKGREY); // -4h
	xDispTo.drawLine( 97+iOffsetX, 16+iOffsetY, 97+iOffsetX,115+iOffsetY,DARKGREY); // -3h
	xDispTo.drawLine(157+iOffsetX, 16+iOffsetY,157+iOffsetX,115+iOffsetY,DARKGREY); // -2h
	xDispTo.drawLine(217+iOffsetX, 16+iOffsetY,217+iOffsetX,115+iOffsetY,DARKGREY); // -1h

	xDispTo.drawLine(  0+iOffsetX, 35+iOffsetY,277+iOffsetX, 35+iOffsetY,DARKGREY); // 200V
	xDispTo.drawLine(  0+iOffsetX, 75+iOffsetY,277+iOffsetX, 75+iOffsetY,DARKGREY); // 100V
}

static constexpr int32_t SYMBOL_WIDTH=32;
static constexpr int32_t SYMBOL_HEIGHT=32;

void disp_symbolClock(symbol::ThreeState xSymbolThreeState)
{
	static constexpr int32_t SYMBOL_CLOCK_X=0;
	static constexpr int32_t SYMBOL_CLOCK_Y=0;
	static const uint16_t* const puBitmaps[]={uBitmap_ClockNG,uBitmap_ClockOK,uBitmap_ClockUnknown};

	M5.Display.pushImage(
		SYMBOL_CLOCK_X,
		SYMBOL_CLOCK_Y,
		SYMBOL_WIDTH,
		SYMBOL_HEIGHT,
		puBitmaps[static_cast<size_t>(xSymbolThreeState)]);
}

void disp_symbolMemory(symbol::TwoState xSymbolTwoState)
{
	static constexpr int32_t SYMBOL_MEMORY_X=48;
	static constexpr int32_t SYMBOL_MEMORY_Y=0;
	static const uint16_t* const puBitmaps[]={uBitmap_MemoryNG,uBitmap_MemoryOK};

	M5.Display.pushImage(
		SYMBOL_MEMORY_X,
		SYMBOL_MEMORY_Y,
		SYMBOL_WIDTH,
		SYMBOL_HEIGHT,
		puBitmaps[static_cast<size_t>(xSymbolTwoState)]);
}

void disp_symbolVolt(symbol::ThreeState xSymbolThreeState)
{
	static constexpr int32_t SYMBOL_VOLT_X=96;
	static constexpr int32_t SYMBOL_VOLT_Y=0;
	static const uint16_t* const puBitmaps[]={uBitmap_VoltNG,uBitmap_VoltOK,uBitmap_VoltUnknown};

	M5.Display.pushImage(
		SYMBOL_VOLT_X,
		SYMBOL_VOLT_Y,
		SYMBOL_WIDTH,
		SYMBOL_HEIGHT,
		puBitmaps[static_cast<size_t>(xSymbolThreeState)]);
}

void disp_symbolLogger(symbol::StopRec xSymbolStopRec)
{
	static constexpr int32_t SYMBOL_LOGGER_X=192;
	static constexpr int32_t SYMBOL_LOGGER_Y=0;
	static const uint16_t* const puBitmaps[]={uBitmap_LoggerStop,uBitmap_LoggerRec};

	M5.Display.pushImage(
		SYMBOL_LOGGER_X,
		SYMBOL_LOGGER_Y,
		SYMBOL_WIDTH,
		SYMBOL_HEIGHT,
		puBitmaps[static_cast<size_t>(xSymbolStopRec)]);
}

void disp_symbolWiFi(symbol::WiFi xSymbolWiFi)
{
	static constexpr int32_t SYMBOL_WIFI_X=240;
	static constexpr int32_t SYMBOL_WIFI_Y=0;
	static const uint16_t* const puBitmaps[]={uBitmap_WiFiNG,uBitmap_WiFi1,uBitmap_WiFi2,uBitmap_WiFi3,uBitmap_WiFi4};

	M5.Display.pushImage(
		SYMBOL_WIFI_X,
		SYMBOL_WIFI_Y,
		SYMBOL_WIDTH,
		SYMBOL_HEIGHT,
		puBitmaps[static_cast<size_t>(xSymbolWiFi)]);
}

void disp_symbolPower(symbol::TwoState xSymbolTwoState)
{
	static constexpr int32_t SYMBOL_POWER_X=288;
	static constexpr int32_t SYMBOL_POWER_Y=0;
	static const uint16_t* const puBitmaps[]={uBitmap_PowerNG,uBitmap_PowerOK};

	M5.Display.pushImage(
		SYMBOL_POWER_X,
		SYMBOL_POWER_Y,
		SYMBOL_WIDTH,
		SYMBOL_HEIGHT,
		puBitmaps[static_cast<size_t>(xSymbolTwoState)]);
}

void disp_dateTime(const m5::rtc_datetime_t& xDateTime)
{
	M5.Display.setTextSize(2);
	M5.Display.setTextColor(CYAN,NAVY);

	if(xDateTime.date.year>=common::YEAR_VALIDFROM)
	{
		M5.Display.setCursor(8,54);
		M5.Display.printf("%04d/%02d/%02d",xDateTime.date.year,xDateTime.date.month,xDateTime.date.date);
		M5.Display.setCursor(32,78);
		M5.Display.printf("%02d:%02d:%02d",xDateTime.time.hours,xDateTime.time.minutes,xDateTime.time.seconds);
		disp_symbolClock(symbol::ThreeState::OK);
	}
	else
	{
		M5.Display.setCursor(8,54);
		M5.Display.print("----/--/--");
		M5.Display.setCursor(32,78);
		M5.Display.print("--:--:--");
		disp_symbolClock(symbol::ThreeState::Unknown);
	}
}

void disp_initial(void)
{
	M5.Display.clear();
	disp_symbolClock(symbol::ThreeState::NG);
	disp_symbolMemory(symbol::TwoState::NG);
	disp_symbolVolt(symbol::ThreeState::NG);
	disp_symbolLogger(symbol::StopRec::Stop);
	disp_symbolWiFi(symbol::WiFi::NG);
	disp_symbolPower(symbol::TwoState::NG);

	M5.Display.fillRoundRect(0,46,320,56,5,NAVY);
	M5.Display.drawRoundRect(0,46,320,56,5,CYAN);

	disp_dateTime({{-1,-1,-1},{-1,-1,-1}});

	M5.Display.setTextSize(4);
	M5.Display.setCursor(156,62);
	M5.Display.setTextColor(WHITE,NAVY);
	M5.Display.print("AC");
	M5.Display.setCursor(210,62);
	M5.Display.print("---");
	M5.Display.setCursor(288,62);
	M5.Display.print("V");

	disp_graphFrame(M5.Display,42,100);
	disp_graphScale(M5.Display,42,100);

	M5.Display.setTextSize(2);
	M5.Display.setTextColor(DARKGREY);

	M5.Display.setCursor(0,128);
	M5.Display.print("200");
	M5.Display.setCursor(0,168);
	M5.Display.print("100");
	M5.Display.setCursor(12,208);
	M5.Display.print("0V");

	M5.Display.setCursor(79-18,224);
	M5.Display.print("-4h");
	M5.Display.setCursor(199-18,224);
	M5.Display.print("-2h");
	M5.Display.setCursor(296,224);
	M5.Display.print("0h");
}

static constexpr int32_t ERRORWINDOW_X=8;
static constexpr int32_t ERRORWINDOW_Y=64;
static constexpr int32_t ERRORWINDOW_W=304;
static constexpr int32_t ERRORWINDOW_H=112;
static constexpr int32_t ERRORWINDOW_R=8;

static constexpr int ERRORWINDOW_BG=MAROON;
static constexpr int ERRORWINDOW_FG=WHITE;

static constexpr int32_t ERRORTEXT_CENTER_X=160;
static constexpr int32_t ERRORTEXT_HEADER_Y=80;
static constexpr int32_t ERRORTEXT_MSG_Y=112;
static constexpr int32_t ERRORTEXT_FOOTER_Y=144;

static constexpr const char *ERRORTEXT_HEADER="----- FATAL ERROR -----";
static constexpr const char *ERRORTEXT_MSG1="RTC unavailable.";
static constexpr const char *ERRORTEXT_MSG2="Memory-card failure.";
static constexpr const char *ERRORTEXT_MSG3="Bad Wi-Fi connection.";
static constexpr const char *ERRORTEXT_FOOTER="(Tap to shut down)";

void disp_error(error::RiseType xRiseType)
{
	M5.Display.fillRoundRect(ERRORWINDOW_X,ERRORWINDOW_Y,ERRORWINDOW_W,ERRORWINDOW_H,ERRORWINDOW_R,ERRORWINDOW_BG);
	M5.Display.drawRoundRect(ERRORWINDOW_X,ERRORWINDOW_Y,ERRORWINDOW_W,ERRORWINDOW_H,ERRORWINDOW_R,ERRORWINDOW_FG);

	M5.Display.setTextSize(2);
	M5.Display.setTextColor(ERRORWINDOW_FG);
	M5.Display.drawCenterString(ERRORTEXT_HEADER,ERRORTEXT_CENTER_X,ERRORTEXT_HEADER_Y);

	switch(xRiseType)
	{
		case error::RiseType::RtcUnavailable:
		{
			M5.Display.drawCenterString(ERRORTEXT_MSG1,ERRORTEXT_CENTER_X,ERRORTEXT_MSG_Y);
		} break;

		case error::RiseType::MemoryCardFailure:
		{
			M5.Display.drawCenterString(ERRORTEXT_MSG2,ERRORTEXT_CENTER_X,ERRORTEXT_MSG_Y);
		} break;

		case error::RiseType::WiFiConnectionFailure:
		{
			M5.Display.drawCenterString(ERRORTEXT_MSG3,ERRORTEXT_CENTER_X,ERRORTEXT_MSG_Y);
		} break;

		default: break;
	}

	M5.Display.drawCenterString(ERRORTEXT_FOOTER,ERRORTEXT_CENTER_X,ERRORTEXT_FOOTER_Y);
}
