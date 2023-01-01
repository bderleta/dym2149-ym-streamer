#pragma once

#include <Windows.h>

#include "YmStream.h"
#include "W32Exception.h"

class YmDevice
{
protected:
	HANDLE hComPort;
	DCB dcb;
	YmFrame pFrame1;
	YmFrame pFrame2;
	DWORD sendBuffer(uint8_t* buffer, DWORD blen);
public:
	YmDevice(LPCWSTR fileName, LPCWSTR dcbDef = NULL);
	~YmDevice();
	void setRegister(uint8_t reg, uint8_t val);
	void sendFrame(YmFrame fd);
	void updateFrame(YmFrame fd);
	void updateTurboFrame(YmFrame nFrame1, YmFrame nFrame2);
	void mute(void);
};

