#pragma once
#include "DeviceHk.h"

class Core
{
public:
	Core();
	~Core();
private:
	DeviceHk m_DeviceHk;
public:
	bool testHk();
};

