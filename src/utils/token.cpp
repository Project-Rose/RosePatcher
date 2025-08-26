#include <algorithm>
#include <format>
#include <string>
#include <wups.h>
#include <wups/storage.h>
#include <coreinit/bsp.h>
#include <coreinit/cache.h>
#include <coreinit/launch.h>
#include <coreinit/mcp.h>
#include <coreinit/title.h>
#include <nn/act/client_cpp.h>
#include <sysapp/launch.h>
#include <sysapp/title.h>

#include "token.hpp"
#include "logger.h"
#include "Notification.hpp"

extern "C" MCPError MCP_GetDeviceId(int handle, char* out);
extern "C" MCPError MCP_GetCompatDeviceId(int handle, char* out);

namespace token {
    std::string currentReplacementToken = "";
    std::vector<std::string> roseKeys(13);

    char codeId[8];
    char serialId[12];

    void initToken() {
        // enable RNG in case we need it.
        srand(time(NULL));

        int handle = MCP_Open();
        MCPSysProdSettings settings alignas(0x40);
        MCPError error = MCP_GetSysProdSettings(handle, &settings);

        if(error) {
            DEBUG_FUNCTION_LINE("MCP_GetSysProdSettings failed");
            ShowNotification("Rose: Failed to get Wii U Settings. You will not be able to launch TVii. Try again via rebooting.");
            return;
        }

        memcpy(codeId, settings.code_id, sizeof(settings.code_id));
        memcpy(serialId, settings.serial_id, sizeof(settings.serial_id));

        MCP_Close(handle);

        unsigned int pid = 0;
        std::string key;

        // FIXME: < or <=?
        for (uint8_t accountSlot = 1; accountSlot < nn::act::GetNumOfAccounts(); accountSlot++) {
			// may help with instability issues?
			OSMemoryBarrier();

            nn::act::GetPrincipalIdEx(&pid, accountSlot);
            DEBUG_FUNCTION_LINE("index %d, pid: %d", accountSlot, pid);
            if(pid == 0) {
                DEBUG_FUNCTION_LINE("PID is 0; account probably not linked to NNID/PNID");
                continue;
            }

            // based on various sources
            // from https://github.com/RiiConnect24/UTag/blob/2287ef6c21e18de77162360cca53c1ccb1b30759/src/main.cpp#L26
            std::string filePath = "fs:/vol/external01/wiiu/rose_key_" + std::to_string(pid) + ".txt";
            FILE *fp = fopen(filePath.c_str(), "r");
            if (!fp) {
                DEBUG_FUNCTION_LINE("File %s found, generating a default.", filePath.c_str());

                fp = fopen(filePath.c_str(), "w");

                key.clear(); // Ensure we are empty

                // open disclosure: made w/ help of chatgpt
                for(int i = 0; i < 17; i++) {
                    char randNum = rand() % 62;
                    if (randNum < 26) {
                        key += 'a' + randNum;  // lowercase letters a-z
                    } else if (randNum < 52) {
                        key += 'A' + (randNum - 26);  // uppercase letters A-Z
                    } else {
                        key += '0' + (randNum - 52);  // digits 0-9
                    }
                }

                key.shrink_to_fit();

                DEBUG_FUNCTION_LINE("newly generated key: %s", key.c_str());
                fputs(key.c_str(), fp);

                fclose(fp);
            } else {
                char buf[20];
                fread(buf, 20, 1, fp);

                buf[17] = '\0'; // null terminate

                key = std::string(buf);
                key.shrink_to_fit();

                DEBUG_FUNCTION_LINE("read key: %s", key.c_str());

                fclose(fp);
            }

            DEBUG_FUNCTION_LINE("slot: %d", accountSlot);
            DEBUG_FUNCTION_LINE("old vec data: %s", roseKeys.at(accountSlot).c_str());
            if(roseKeys.at(accountSlot).empty()) {
                roseKeys.insert(roseKeys.begin() + accountSlot, key);
                DEBUG_FUNCTION_LINE("new vec data: %s", roseKeys.at(accountSlot).c_str());
            } else {
                DEBUG_FUNCTION_LINE("not empty, was %s", roseKeys.at(accountSlot).c_str());
            }
        }

        // when we are done:
        roseKeys.shrink_to_fit();

        updCurrentToken();
    }

    // This sounds pointless, but if we don't want to double insert things
    void resetTokens() {
        DEBUG_FUNCTION_LINE("Resetting tokens");
        roseKeys.clear();
        roseKeys.shrink_to_fit();

        roseKeys.reserve(13);
    }

    void updCurrentToken() {
        DEBUG_FUNCTION_LINE("Getting token from slot %d", nn::act::GetSlotNo());

        currentReplacementToken = std::format("[{}, {}, {}]", roseKeys.at(nn::act::GetSlotNo()), codeId, serialId);
        reverse(std::next(currentReplacementToken.begin()), std::prev(currentReplacementToken.end()));

        DEBUG_FUNCTION_LINE("result: %s", currentReplacementToken.c_str());
    }
} // namespace token
