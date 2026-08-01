#include <Arduino.h>
#include <M5Unified.h>

#include "bitmaps.h"
#include "common.h"
#include "disp.h"
#include "error.h"
#include "task_adc.h"

static constexpr int32_t GRAPH_WIDTH=278;
static constexpr int32_t GRAPH_HEIGHT=100;

static constexpr int32_t GRAPH_OFFSET_X=42;
static constexpr int32_t GRAPH_OFFSET_Y=116;

static LGFX_Sprite xSprite_graphArea(&M5.Display);

static void disp_graphFrame(LovyanGFX& xDispTo,int32_t iOffsetX,int32_t iOffsetY)
{
	xDispTo.drawRect(iOffsetX,iOffsetY,GRAPH_WIDTH,GRAPH_HEIGHT,DARKGREY);
}

static constexpr int COLOR_HASH=DARKGREY;

// Required: iY_from<=iY_to
static void drawVertHash(LovyanGFX& xDispTo,int32_t iX,int32_t iY_from,int32_t iY_to)
{
	int32_t iY;

	for(iY=iY_from;iY<=iY_to;iY+=8) xDispTo.drawPixel(iX,iY,COLOR_HASH);
}

// Required: iX_from<=iX_to
static void drawHorizHash(LovyanGFX& xDispTo,int32_t iX_from,int32_t iX_to,int32_t iY)
{
	int32_t iX;

	for(iX=iX_from;iX<=iX_to;iX+=8) xDispTo.drawPixel(iX,iY,COLOR_HASH);
}

static void disp_graphScale(LovyanGFX& xDispTo,int32_t iOffsetX,int32_t iOffsetY)
{
/*
	xDispTo.drawLine( 37+iOffsetX,iOffsetY, 37+iOffsetX,(GRAPH_HEIGHT-1)+iOffsetY,DARKGREY); // -4h
	xDispTo.drawLine( 97+iOffsetX,iOffsetY, 97+iOffsetX,(GRAPH_HEIGHT-1)+iOffsetY,DARKGREY); // -3h
	xDispTo.drawLine(157+iOffsetX,iOffsetY,157+iOffsetX,(GRAPH_HEIGHT-1)+iOffsetY,DARKGREY); // -2h
	xDispTo.drawLine(217+iOffsetX,iOffsetY,217+iOffsetX,(GRAPH_HEIGHT-1)+iOffsetY,DARKGREY); // -1h

	xDispTo.drawLine(iOffsetX,19+iOffsetY,(GRAPH_WIDTH-1)+iOffsetX,19+iOffsetY,DARKGREY); // 200V
	xDispTo.drawLine(iOffsetX,59+iOffsetY,(GRAPH_WIDTH-1)+iOffsetX,59+iOffsetY,DARKGREY); // 100V
*/
	drawVertHash(xDispTo, 37+iOffsetX,iOffsetY,(GRAPH_HEIGHT-1)+iOffsetY); // -4h
	drawVertHash(xDispTo, 97+iOffsetX,iOffsetY,(GRAPH_HEIGHT-1)+iOffsetY); // -3h
	drawVertHash(xDispTo,157+iOffsetX,iOffsetY,(GRAPH_HEIGHT-1)+iOffsetY); // -2h
	drawVertHash(xDispTo,217+iOffsetX,iOffsetY,(GRAPH_HEIGHT-1)+iOffsetY); // -1h

	drawHorizHash(xDispTo,iOffsetX,(GRAPH_WIDTH-1)+iOffsetX,19+iOffsetY); // 200V
	drawHorizHash(xDispTo,iOffsetX,(GRAPH_WIDTH-1)+iOffsetX,59+iOffsetY); // 100V
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

bool disp_dateTime(const m5::rtc_datetime_t& xDateTime)
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
		return(true);
	}
	else
	{
		M5.Display.setCursor(8,54);
		M5.Display.print("----/--/--");
		M5.Display.setCursor(32,78);
		M5.Display.print("--:--:--");
		disp_symbolClock(symbol::ThreeState::Unknown);
		return(false);
	}
}

static void sub_drawGraph(unsigned char ucVoltRMS)
{
	static struct
	{
		unsigned char ucY_avg;
		unsigned char ucY_max;
		unsigned char ucY_min;
	} xGraph[GRAPH_WIDTH]={};

	static unsigned short usSum;
	static unsigned char ucMax,ucMin;
	static unsigned char ucCount=0U;

	constexpr unsigned char STATISTIC_PERIOD=60U;
	constexpr unsigned char IS_DATA_EXISTS=0x80U;

	int i;

	if(ucCount==0U)
	{
		usSum=0U;
		ucMax=0U;
		ucMin=255U;
	}

	usSum+=(unsigned short)ucVoltRMS;
	if(ucMax<ucVoltRMS) ucMax=ucVoltRMS;
	if(ucMin>ucVoltRMS) ucMin=ucVoltRMS;

	if(++ucCount>=STATISTIC_PERIOD)
	{
		ucCount=0U;

		for(i=1;i<=(GRAPH_WIDTH-1);i++)
		{
			xGraph[i-1]=xGraph[i];
		}
		xGraph[GRAPH_WIDTH-1].ucY_avg=(unsigned char)(usSum*2U/(STATISTIC_PERIOD*5U)); // *2/5 ===== *0.4
		xGraph[GRAPH_WIDTH-1].ucY_max=(unsigned char)((unsigned short)ucMax*2U/5U); // *2/5 ===== *0.4
		xGraph[GRAPH_WIDTH-1].ucY_min=(unsigned char)((unsigned short)ucMin*2U/5U); // *2/5 ===== *0.4

		if(xGraph[GRAPH_WIDTH-1].ucY_avg>(GRAPH_HEIGHT-1)) xGraph[GRAPH_WIDTH-1].ucY_avg=(GRAPH_HEIGHT-1);
		if(xGraph[GRAPH_WIDTH-1].ucY_max>(GRAPH_HEIGHT-1)) xGraph[GRAPH_WIDTH-1].ucY_max=(GRAPH_HEIGHT-1);
		if(xGraph[GRAPH_WIDTH-1].ucY_min>(GRAPH_HEIGHT-1)) xGraph[GRAPH_WIDTH-1].ucY_min=(GRAPH_HEIGHT-1);

		xGraph[GRAPH_WIDTH-1].ucY_avg=(GRAPH_HEIGHT-1)-xGraph[GRAPH_WIDTH-1].ucY_avg;
		xGraph[GRAPH_WIDTH-1].ucY_max=(GRAPH_HEIGHT-1)-xGraph[GRAPH_WIDTH-1].ucY_max;
		xGraph[GRAPH_WIDTH-1].ucY_min=(GRAPH_HEIGHT-1)-xGraph[GRAPH_WIDTH-1].ucY_min;

		xGraph[GRAPH_WIDTH-1].ucY_avg|=IS_DATA_EXISTS;

		xSprite_graphArea.fillSprite(BLACK);
		disp_graphFrame(xSprite_graphArea,0,0);
		disp_graphScale(xSprite_graphArea,0,0);

		for(i=0;i<=(GRAPH_WIDTH-1);i++)
		{
			if(xGraph[i].ucY_avg & IS_DATA_EXISTS)
			{
				xSprite_graphArea.drawLine(i,xGraph[i].ucY_avg & ~IS_DATA_EXISTS,i,xGraph[i].ucY_max,RED);
				xSprite_graphArea.drawPixel(i,xGraph[i].ucY_max,PINK);

				xSprite_graphArea.drawLine(i,xGraph[i].ucY_avg & ~IS_DATA_EXISTS,i,xGraph[i].ucY_min,BLUE);
				xSprite_graphArea.drawPixel(i,xGraph[i].ucY_min,CYAN);

				xSprite_graphArea.drawPixel(i,xGraph[i].ucY_avg & ~IS_DATA_EXISTS,WHITE);
			}
		}
		xSprite_graphArea.pushSprite(GRAPH_OFFSET_X,GRAPH_OFFSET_Y);
	}
}

void disp_currentResult(const struct task_adc::Result& xResult)
{
	if(xResult.fAdaptiveOffset<common::NORMAL_ADC_OFFSET_MIN ||
	   xResult.fAdaptiveOffset>common::NORMAL_ADC_OFFSET_MAX)
	{
		disp_symbolVolt(symbol::ThreeState::Unknown);
	}
	else disp_symbolVolt(symbol::ThreeState::OK);

	M5.Display.setTextSize(4);
	M5.Display.setTextColor(WHITE,NAVY);
	M5.Display.setCursor(210,62);
	M5.Display.printf("%3u",xResult.ucVoltRMS);

	sub_drawGraph(xResult.ucVoltRMS);
}

void disp_initialize(void)
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
	M5.Display.setTextColor(WHITE,NAVY);
	M5.Display.setCursor(156,62);
	M5.Display.print("AC");
	M5.Display.setCursor(210,62);
	M5.Display.print("---");
	M5.Display.setCursor(288,62);
	M5.Display.print("V");

	disp_graphFrame(M5.Display,GRAPH_OFFSET_X,GRAPH_OFFSET_Y);
	disp_graphScale(M5.Display,GRAPH_OFFSET_X,GRAPH_OFFSET_Y);

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

	xSprite_graphArea.setColorDepth(16);
	xSprite_graphArea.createSprite(GRAPH_WIDTH,GRAPH_HEIGHT);
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
