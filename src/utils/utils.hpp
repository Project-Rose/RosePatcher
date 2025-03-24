#pragma once
#include <wups.h>

#include <coreinit/title.h>
#include <coreinit/mcp.h>
#include <coreinit/userconfig.h>
#include <nn/swkbd.h>

#include <cstring>
#include <string>
#include <vector>
#include <optional>

namespace utils {
    bool isVinoClientID(const char *client_id);
    bool isVinoTitleID(uint32_t title_id);

    bool isWiiUMenuTitleID(uint32_t title_id, bool includeJPN = true);

    MCPSystemVersion getSystemVersion();

    char getConsoleRegion();
    bool isJapanConsole();
    bool isUSAConsole();
    bool isEuropeConsole();

    bool is555OrHigher();

    nn::swkbd::LanguageType getConsoleLanguage();
}; // namespace utils