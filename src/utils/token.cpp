#include <algorithm>
#include <format>
#include <string>
#include <wups.h>
#include <wups/storage.h>
#include <coreinit/bsp.h>
#include <coreinit/launch.h>
#include <coreinit/mcp.h>
#include <coreinit/title.h>
#include <nn/act/client_cpp.h>
#include <sysapp/launch.h>
#include <sysapp/title.h>

#include "token.hpp"
#include "utils/utils.hpp"
#include "utils/logger.h"
#include "utils/Notification.hpp"

extern "C" MCPError MCP_GetDeviceId(int handle, char* out);
extern "C" MCPError MCP_GetCompatDeviceId(int handle, char* out);

namespace token {
    std::string currentReplacementToken = "";

    char replacementToken1[20];
    char replacementToken2[20];
    char replacementToken3[20];
    char replacementToken4[20];
    char replacementToken5[20];
    char replacementToken6[20];
    char replacementToken7[20];
    char replacementToken8[20];
    char replacementToken9[20];
    char replacementToken10[20];
    char replacementToken11[20];
    char replacementToken12[20];
    char codeId[8];
    char serialId[12];

    void setReplacementToken(char token[20], nn::act::SlotNo slot) {
        // really stupid thing i am gonna do but yeahaha
        switch (slot) {
            case 1:
                strcpy(replacementToken1, token);
                break;
            
            case 2:
                strcpy(replacementToken2, token);
                break;
            
            case 3:
                strcpy(replacementToken3, token);
                break;
            
            case 4:
                strcpy(replacementToken4, token);
                break;
            
            case 5:
                strcpy(replacementToken5, token);
                break;
            
            case 6:
                strcpy(replacementToken6, token);
                break;
            
            case 7:
                strcpy(replacementToken7, token);
                break;
            
            case 8:
                strcpy(replacementToken8, token);
                break;
            
            case 9:
                strcpy(replacementToken9, token);
                break;
            
            case 10:
                strcpy(replacementToken10, token);
                break;
            
            case 11:
                strcpy(replacementToken11, token);
                break;
            
            case 12:
                strcpy(replacementToken12, token);
                break;
            
            default:
                break;
        }
    }
    
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
        char key[20] = { '\0' };
        for (size_t i = 1; i < 12; i++) {
            if (!nn::act::IsSlotOccupied(i)) {
                // No more accounts
                DEBUG_FUNCTION_LINE("Slot %d not occupied", i);
                return;
            }

            nn::act::GetPrincipalIdEx(&pid, i);
            DEBUG_FUNCTION_LINE("index %d, pid: %d", i, pid);
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

                std::string newKey = ""; // Ensure reset

                // open disclosure: made w/ help of chatgpt
                for(int i = 0; i < 17; i++) {
                    int randNum = rand() % 62;
                    if (randNum < 26) {
                        newKey += 'a' + randNum;  // lowercase letters a-z
                    } else if (randNum < 52) {
                        newKey += 'A' + (randNum - 26);  // uppercase letters A-Z
                    } else {
                        newKey += '0' + (randNum - 52);  // digits 0-9
                    }
                }

                newKey.shrink_to_fit();

                DEBUG_FUNCTION_LINE("newly generated key: %s", newKey.c_str());
                fputs(newKey.c_str(), fp);
                strcpy(key, newKey.c_str());

                newKey = "";

                fclose(fp);
            } else {
                fread(key, 20, 1, fp);
                DEBUG_FUNCTION_LINE("read key: %s", key);

                fclose(fp);
            }

            setReplacementToken(key, i);
        }
    }

    void updCurrentToken() {
        DEBUG_FUNCTION_LINE("Getting token from slot %d", nn::act::GetSlotNo());
        switch (nn::act::GetSlotNo()) {
            case 1:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken1, codeId, serialId);
                break;
            
            case 2:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken2, codeId, serialId);
                break;
            
            case 3:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken3, codeId, serialId);
                break;
            
            case 4:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken4, codeId, serialId);
                break;
            
            case 5:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken5, codeId, serialId);
                break;
            
            case 6:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken6, codeId, serialId);
                break;
            
            case 7:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken7, codeId, serialId);
                break;
            
            case 8:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken8, codeId, serialId);
                break;
            
            case 9:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken9, codeId, serialId);
                break;
            
            case 10:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken10, codeId, serialId);
                break;
            
            case 11:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken11, codeId, serialId);
                break;
            
            case 12:
                currentReplacementToken = std::format("[{}, {}, {}]", replacementToken12, codeId, serialId);
                break;
            
            default:
                break;
        }
        reverse(std::next(currentReplacementToken.begin()), std::prev(currentReplacementToken.end()));
        DEBUG_FUNCTION_LINE("result: %s", currentReplacementToken.c_str());
    }
} // namespace token