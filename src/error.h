#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

namespace error
{
	enum class RiseType : uint8_t
	{
		RtcUnavailable,
		MemoryCardFailure,
		WiFiConnectionFailure
	};

	enum class HandleState : uint8_t
	{
		NotHandling,
		WaitingTouch,
		WaitingRelease
	};
}

void error_rise(error::RiseType xRiseType);
bool error_handle(void);

#endif // #ifndef ERROR_H
