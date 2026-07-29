#include <Arduino.h>
#include <M5Unified.h>

#include "disp.h"
#include "error.h"

static error::HandleState xErrorHandleState=error::HandleState::NotHandling;

void error_rise(error::RiseType xRiseType)
{
	disp_error(xRiseType);
	xErrorHandleState=error::HandleState::WaitingTouch;
}

bool error_handle(void)
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
			if(!M5.Touch.getCount()) M5.Power.powerOff();
			return(true);
		} break;

		case error::HandleState::NotHandling:
		default:
		{
			return(false);
		} break;
	}
}
