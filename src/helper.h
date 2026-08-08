#ifndef HELPER_H
#define HELPER_H

namespace helper
{
	void updateStatusWiFi(void);
	bool powerOK/*updateStatusPower*/(void);
	bool isValidFilename(const char *pcFilename);
}

#endif // #ifndef HELPER_H
