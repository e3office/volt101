#include <Arduino.h>
#include <M5Unified.h>
#include <SD.h>

#include "appconf.h"

struct AppConf xAppConf;

static size_t appConf_readLine(File &xFile,char cBuffer[])
{
	size_t i;
	int iByteRead;

	for(i=0;i<LENGTH_FIELD_APPCONF;i++)
	{
		iByteRead=xFile.read();
		if(iByteRead<0)
		{
			cBuffer[i]='\0';
			return(i);
		}
		if(iByteRead<0x20)
		{
			cBuffer[i]='\0';
			for(;;)
			{
				iByteRead=xFile.peek();
				if(iByteRead<0 || iByteRead>=0x20) break;
				xFile.read(); // discard trailing control-code
			}
			return(i);
		}
		cBuffer[i]=(char)iByteRead;
	}
	cBuffer[i]='\0';
	for(;;)
	{
		iByteRead=xFile.peek();
		if(iByteRead<0x20) break;
		xFile.read(); // discard trailing non-control-code
	}
	for(;;)
	{
		iByteRead=xFile.peek();
		if(iByteRead<0 || iByteRead>=0x20) break;
		xFile.read(); // discard trailing control-code
	}
	return(i);
}

bool appConf_load(void)
{
	File xFile;
	bool bResult;

	xFile=SD.open(PATH_APPCONF,FILE_READ);
	if(!xFile) return(false);

	bResult=true;
	if(!appConf_readLine(xFile,xAppConf.cWifiSsid)) bResult=false;
	if(!appConf_readLine(xFile,xAppConf.cWifiPass)) bResult=false;
	if(!appConf_readLine(xFile,xAppConf.cNtpServer)) bResult=false;

	xFile.close();
	return(bResult);
}
