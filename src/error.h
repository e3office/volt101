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

	void shutdownSafely(void);

	void rise(error::RiseType xRiseType);
	bool handle(void);
}

#endif // #ifndef ERROR_H
