#include <wups.h>

#include <coreinit/mcp.h>
#include <coreinit/title.h>

#include <cstring>
#include <string>
#include <vector>

#include "../config.hpp"
#include "utils.hpp"

#define VINO_TITLE_ID_JP 0x000500301001300A
#define VINO_CLIENT_ID_JP "547e315c1966905040e2d48dff24439a"

#define VINO_TITLE_ID_US 0x000500301001310A
#define VINO_CLIENT_ID_US "87a44dad436407e4ec47ad42ed68bf8c"

#define VINO_TITLE_ID_EU 0x000500301001320A
#define VINO_CLIENT_ID_EU "8bc9387d0797e003c3210acfae01e109"

namespace utils {
MCPSystemVersion version = {.major = 0, .minor = 0, .patch = 0, .region = 'N'};

bool isVinoClientID(const char *client_id) {
  return strcmp(client_id, VINO_CLIENT_ID_JP) == 0 ||
         strcmp(client_id, VINO_CLIENT_ID_US) == 0 ||
         strcmp(client_id, VINO_CLIENT_ID_EU) == 0;
}

bool isVinoTitleID(uint32_t title_id) {
  return title_id == VINO_TITLE_ID_JP || title_id == VINO_TITLE_ID_US ||
         title_id == VINO_TITLE_ID_EU;
}

MCPSystemVersion getSystemVersion() {
  if (version.major != 0 && version.minor != 0 && version.patch != 0 &&
      version.region != 'N') {
    return version;
  }
  int mcp = MCP_Open();
  int ret = MCP_GetSystemVersion(mcp, &version);
  if (ret < 0) {
    version = {.major = 0, .minor = 0, .patch = 0, .region = 'N'};
  }
  MCP_Close(mcp);
  return version;
}

char getConsoleRegion() { return getSystemVersion().region; }

bool isJapanConsole() {
  return getConsoleRegion() == 'J' || config::forceJPNconsole;
}

bool isUSAConsole() { return getConsoleRegion() == 'U'; }

bool isEuropeConsole() { return getConsoleRegion() == 'E'; }

bool is555OrHigher() {
  return getSystemVersion().major == 5 && getSystemVersion().minor == 5 &&
         getSystemVersion().patch >= 5 &&
         (getSystemVersion().region == 'U' ||
          getSystemVersion().region == 'E' || getSystemVersion().region == 'J');
}
}; // namespace utils