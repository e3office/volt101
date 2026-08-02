#include <Arduino.h>
#include <math.h>

#include "common.h"
#include "task_adc.h"

namespace task_adc
{
	static constexpr int PERIOD_RMS=100;
	static constexpr float F_SQRT2=1.41421356F;
	static constexpr float SCALE_FACTOR=100.0F/(208.333F/2.0F/F_SQRT2);

	static hw_timer_t *pxHardwareTimer;

	static TaskHandle_t xTaskHandle=nullptr;

	struct ResultIntermed
	{
		float fVoltSumSquare;
		float fAdaptiveOffset;
	};

	static ResultIntermed xResultIntermed;
	static portMUX_TYPE xMutex=portMUX_INITIALIZER_UNLOCKED;

	void getResult(struct Result *pxResult)
	{
		struct ResultIntermed xResultIntermedCopy;
		float fVoltRMS;

		portENTER_CRITICAL(&xMutex);
		xResultIntermedCopy=xResultIntermed;
		portEXIT_CRITICAL(&xMutex);

		fVoltRMS=SCALE_FACTOR*sqrtf(xResultIntermedCopy.fVoltSumSquare/PERIOD_RMS);
		pxResult->ucVoltRMS=(fVoltRMS>=255.0F? 255U : (unsigned char)fVoltRMS);
		pxResult->fAdaptiveOffset=xResultIntermedCopy.fAdaptiveOffset;
	}

	static void vTaskMain(void *pvParameters)
	{
		constexpr float IIR_GAIN_B=0.005F;
		constexpr float IIR_GAIN_A=1.0F-IIR_GAIN_B;

		static float fAdaptiveOffset=1650.0F; // 3300mV/2
		static float fVoltSumSquare=0.0F;
		static int iCountCumulate=0;

		uint32_t ulAnalogMilliVolts;
		float fOffsetRemoved;

		for(;;)
		{
			ulTaskNotifyTake(pdTRUE,portMAX_DELAY);

			ulAnalogMilliVolts=analogReadMilliVolts(common::PIN_AI_VOLT);

			fAdaptiveOffset*=IIR_GAIN_A;
			fAdaptiveOffset+=IIR_GAIN_B*(float)ulAnalogMilliVolts;

			fOffsetRemoved=(float)ulAnalogMilliVolts-fAdaptiveOffset;

			if(iCountCumulate>=PERIOD_RMS)
			{
				portENTER_CRITICAL(&xMutex);
				xResultIntermed.fVoltSumSquare=fVoltSumSquare;
				xResultIntermed.fAdaptiveOffset=fAdaptiveOffset;
				portEXIT_CRITICAL(&xMutex);

				fVoltSumSquare=0.0F;
				iCountCumulate=0;
			}

			fVoltSumSquare+=fOffsetRemoved*fOffsetRemoved;
			iCountCumulate++;
		}
	}

	static void IRAM_ATTR timerCallbackFunc(void)
	{
		BaseType_t xHigherPriorityTaskWoken=pdFALSE;

		if(xTaskHandle!=NULL)
		{
			vTaskNotifyGiveFromISR(xTaskHandle,&xHigherPriorityTaskWoken);
			if(xHigherPriorityTaskWoken==pdTRUE) portYIELD_FROM_ISR();
		}
	}

	void initializeTask(void)
	{
		constexpr uint32_t CPU_FREQ=80000000UL; // 80MHz
		constexpr uint32_t TIMER_BASE=1000000UL; // 1MHz
		constexpr uint32_t ALARM_CYCLE=1000UL; // 1kHz

		constexpr uint8_t TIMER_NUM=3U;
		constexpr uint16_t TIMER_DIV=(uint16_t)(CPU_FREQ/TIMER_BASE);
		constexpr bool TIMER_COUNTUP=true;

		constexpr bool TIMER_INTR_BYPOSEDGE=true;
		constexpr uint64_t ALARM_INTR_AT=(uint64_t)(TIMER_BASE/ALARM_CYCLE);
		constexpr bool ALARM_AUTORELOAD=true;

		if(xTaskHandle==nullptr)
		{
			xTaskCreatePinnedToCore(vTaskMain,"Task_ADC",2048,NULL,21,&xTaskHandle,0);
		}

		pxHardwareTimer=timerBegin(TIMER_NUM,TIMER_DIV,TIMER_COUNTUP);
		timerAttachInterrupt(pxHardwareTimer,timerCallbackFunc,TIMER_INTR_BYPOSEDGE);
		timerAlarmWrite(pxHardwareTimer,ALARM_INTR_AT,ALARM_AUTORELOAD);
		timerAlarmEnable(pxHardwareTimer);
	}
}
