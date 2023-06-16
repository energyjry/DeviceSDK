#include "Core.h"

Core::Core()
{
}

Core::~Core()
{
}

bool Core::testHk()
{
    bool ret;
    ret = m_DeviceHk.Login("10.0.16.111", 8000, "admin", "l1234567");
    if (!m_DeviceHk.Login("10.0.16.111", 8000, "admin", "l1234567")) {
        return false;
    }
    if (!m_DeviceHk.StartPlay(1)) {
        return false;
    }
    return true;
}
