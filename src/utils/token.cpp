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

#include "config.hpp"
#include "token.hpp"
#include "utils/utils.hpp"
#include "utils/logger.h"
#include "utils/Notification.hpp"

extern "C" MCPError MCP_GetDeviceId(int handle, char *out);
extern "C" MCPError MCP_GetCompatDeviceId(int handle, char *out);
extern "C" uint32_t nnactGetCountryEx(char *country, uint8_t slot) asm("GetCountryEx__Q2_2nn3actFPcUc");

namespace token
{
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
    char countryId[3];
    nn::act::PrincipalId pid{};

    /**
     * @brief Calculates the maximum Base64 encoded length for a given number of bytes.
     * @details Base64 expands every 3 bytes into 4 characters, plus padding.
     * Add 1 extra for null terminator.
     * @param n Number of input bytes.
     * @return Maximum required output size in bytes.
     */
#define BASE64_ENCODED_SIZE(n) ((((n) + 2) / 3) * 4 + 1)

    /**
     * @brief Encodes a binary buffer into Base64 text.
     * @param input Pointer to raw input bytes.
     * @param len Number of bytes in input.
     * @param output Pointer to destination buffer (must be at least BASE64_ENCODED_SIZE(len)).
     */
    static void base64_encode(const unsigned char *input, size_t len, char *output)
    {
        static const char cBase64Alphabet[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        size_t outIndex = 0;
        size_t i = 0;

        while (i + 2 < len)
        {
            // Take 3 bytes and split into 4 groups of 6 bits.
            int triple = (input[i] << 16) | (input[i + 1] << 8) | input[i + 2];
            output[outIndex++] = cBase64Alphabet[(triple >> 18) & 0x3F];
            output[outIndex++] = cBase64Alphabet[(triple >> 12) & 0x3F];
            output[outIndex++] = cBase64Alphabet[(triple >> 6) & 0x3F];
            output[outIndex++] = cBase64Alphabet[triple & 0x3F];
            i += 3;
        }

        // Handle remaining 1 or 2 bytes with padding.
        if (i < len)
        {
            int triple = input[i] << 16;
            if (i + 1 < len)
            {
                triple |= input[i + 1] << 8;
            }

            output[outIndex++] = cBase64Alphabet[(triple >> 18) & 0x3F];
            output[outIndex++] = cBase64Alphabet[(triple >> 12) & 0x3F];

            if (i + 1 < len)
            {
                output[outIndex++] = cBase64Alphabet[(triple >> 6) & 0x3F];
                output[outIndex++] = '=';
            }
            else
            {
                output[outIndex++] = '=';
                output[outIndex++] = '=';
            }
        }

        // Null terminate.
        output[outIndex] = '\0';
    }

    static void obfuscate_string(char *str, size_t len)
    {
        // FIXME: use a build-time config header for this
        static const char secret[] = "placeholder";
#define SECRET_LEN (sizeof(secret) - 1) // exclude null terminator

        for (size_t i = 0; i < len; ++i)
        {
            str[i] ^= secret[i % SECRET_LEN]; // XOR each byte with secret
        }
    }

    void setReplacementToken(char token[20], nn::act::SlotNo slot)
    {
        // really stupid thing i am gonna do but yeahaha
        switch (slot)
        {
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

    void initToken()
    {
        // enable RNG in case we need it.
        srand(time(NULL));

        int handle = MCP_Open();
        MCPSysProdSettings settings alignas(0x40);
        MCPError error = MCP_GetSysProdSettings(handle, &settings);

        if (error)
        {
            DEBUG_FUNCTION_LINE("MCP_GetSysProdSettings failed");
            ShowNotification("Failed to get Wii U Settings. You will not be able to launch TVii. Try again via rebooting.");
            return;
        }

        memcpy(codeId, settings.code_id, sizeof(settings.code_id));
        memcpy(serialId, settings.serial_id, sizeof(settings.serial_id));

        MCP_Close(handle);

        unsigned int pid = 0;
        char key[20] = {'\0'};
        for (size_t i = 1; i <= 12; i++)
        {
            if (!nn::act::IsSlotOccupied(i))
            {
                DEBUG_FUNCTION_LINE("Slot %d not occupied", i);
                continue;
            }

            // may help with instability issues?
            OSMemoryBarrier();

            nn::act::GetPrincipalIdEx(&pid, i);
            DEBUG_FUNCTION_LINE("index %d, pid: %d", i, pid);
            if (pid == 0)
            {
                DEBUG_FUNCTION_LINE("PID is 0; account probably not linked to NNID/PNID");
                continue;
            }

            // based on various sources
            // from https://github.com/RiiConnect24/UTag/blob/2287ef6c21e18de77162360cca53c1ccb1b30759/src/main.cpp#L26
            std::string filePath = "fs:/vol/external01/wiiu/rose_key_" + std::to_string(pid) + ".txt";
            FILE *fp = fopen(filePath.c_str(), "r");
            if (!fp)
            {
                DEBUG_FUNCTION_LINE("File %s found, generating a default.", filePath.c_str());

                fp = fopen(filePath.c_str(), "w");

                std::string newKey = ""; // Ensure reset

                // open disclosure: made w/ help of chatgpt
                for (int i = 0; i < 17; i++)
                {
                    int randNum = rand() % 62;
                    if (randNum < 26)
                    {
                        newKey += 'a' + randNum; // lowercase letters a-z
                    }
                    else if (randNum < 52)
                    {
                        newKey += 'A' + (randNum - 26); // uppercase letters A-Z
                    }
                    else
                    {
                        newKey += '0' + (randNum - 52); // digits 0-9
                    }
                }

                newKey.shrink_to_fit();

                DEBUG_FUNCTION_LINE("newly generated key: %s", newKey.c_str());
                fputs(newKey.c_str(), fp);
                strcpy(key, newKey.c_str());

                newKey = "";

                fclose(fp);
            }
            else
            {
                fgets(key, 20, fp);
                DEBUG_FUNCTION_LINE("read key: %s", key);

                fclose(fp);
            }

            setReplacementToken(key, i);
        }
    }

    void updCurrentToken()
    {
        const nn::act::SlotNo slotNo = nn::act::GetSlotNo();
        nn::act::GetPrincipalIdEx(&pid, slotNo);
        nnactGetCountryEx(&countryId[0], slotNo);

        const char *replacementTokens[] = {
            replacementToken1, replacementToken2, replacementToken3,
            replacementToken4, replacementToken5, replacementToken6,
            replacementToken7, replacementToken8, replacementToken9,
            replacementToken10, replacementToken11, replacementToken12};

        if (slotNo < 1 || slotNo > 12)
            return;

        currentReplacementToken = std::format(
            "{}, {}, {}, {}, {}, {}",
            pid,
            replacementTokens[slotNo - 1],
            codeId,
            serialId,
            countryId,
            VERSION);

        obfuscate_string(
            currentReplacementToken.data(),
            currentReplacementToken.size());

        // Base64 encode
        char base64Buffer[BASE64_ENCODED_SIZE(256)];
        base64_encode(
            reinterpret_cast<const unsigned char *>(currentReplacementToken.data()),
            currentReplacementToken.size(),
            base64Buffer);

        currentReplacementToken = base64Buffer;
    }
} // namespace token
