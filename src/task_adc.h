#ifndef TASK_ADC_H
#define TASK_ADC_H

namespace task_adc
{
	struct Result
	{
		unsigned char ucVoltRMS;
		float fAdaptiveOffset;
	};

	void getResult(struct Result *pxResult);
	void initializeTask(void);
}

#endif // #ifndef TASK_ADC_H
