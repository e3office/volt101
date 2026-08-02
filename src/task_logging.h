#ifndef TASK_LOGGING_H
#define TASK_LOGGING_H

namespace task_logging
{
	extern volatile bool bBusy;
	extern volatile bool bFatal;

	void pushResult(const struct task_adc::Result& xResult,
	                const m5::rtc_datetime_t& xCurrentDateTime);

	void initializeTask(void);
}

#endif // #ifndef TASK_LOGGING_H
