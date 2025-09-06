#pragma once
#include <wups.h>

#include <coreinit/title.h>
#include <coreinit/mcp.h>

#include <cstring>
#include <string>
#include <vector>

namespace utils {
    bool isVinoClientID(const char *client_id);
    bool isVinoTitleID(uint32_t title_id);

    MCPSystemVersion getSystemVersion();

    char getConsoleRegion();
    bool isJapanConsole();
    bool isUSAConsole();
    bool isEuropeConsole();

    bool is555OrHigher();
}; // namespace utils