#include <Arduino.h>
#include <M5Unified.h>

#include "disp.h"
#include "error.h"
//#include "task_logging.h"

namespace error
{
	static HandleState xErrorHandleState=HandleState::NotHandling;

	void shutdownSafely(void)
	{
		/*
		constexpr unsigned short SUBSECONDS_TO_SHUTDOWN_FORCE=300U; // 300*0.1s=30s
		unsigned short usTimerSubseconds;

		for(usTimerSubseconds=SUBSECONDS_TO_SHUTDOWN_FORCE;usTimerSubseconds && task_logging::bBusy;usTimerSubseconds--)
		{
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		*/
		M5.Power.powerOff();

		for(;;) vTaskDelay(pdMS_TO_TICKS(1000));
	}

	void rise(error::RiseType xRiseType)
	{
		disp_error(xRiseType);
		xErrorHandleState=error::HandleState::WaitingTouch;
	}

	bool handle(void)
	{
		switch(xErrorHandleState)
		{
			case error::HandleState::WaitingTouch:
			{
				M5.update();
				if(M5.Touch.getCount()) xErrorHandleState=error::HandleState::WaitingRelease;
				return(true);
			} break;

			case error::HandleState::WaitingRelease:
			{
				M5.update();
				if(!M5.Touch.getCount()) shutdownSafely();
				return(true);
			} break;

			case error::HandleState::NotHandling:
			default:
			{
				return(false);
			} break;
		}
	}
}
