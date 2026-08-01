#include <Arduino.h>
#include <M5Unified.h>

#include "disp.h"
#include "error.h"

namespace error
{
	static HandleState xErrorHandleState=HandleState::NotHandling;

	void shutdownSafely(void)
	{
		M5.Power.powerOff();
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
