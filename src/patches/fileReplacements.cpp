#include <wups.h>
#include <coreinit/filesystem.h>

#include "../config.hpp"
#include "../utils/Notification.hpp"
#include "../utils/logger.h"
#include "../utils/patch.hpp"

// included at runtime
#include "rose_config_txt.h"
#include "UsEnglish_xml.h"
#include "UsFrench_xml.h"
#include "UsPortuguese_xml.h"
#include "UsSpanish_xml.h"
#include "EuDutch_xml.h"
#include "EuEnglish_xml.h"
#include "EuFrench_xml.h"
#include "EuGerman_xml.h"
#include "EuItalian_xml.h"
#include "EuPortuguese_xml.h"
#include "EuRussian_xml.h"
#include "EuSpanish_xml.h"

#define VINO_CONFIG_PATH "/vol/content/vino_config.txt"

const char* WWP_PATH = nullptr;

int getWWPPath() {
    if (utils::getConsoleRegion() == 'U' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::English) {
        WWP_PATH = "/vol/content/UsEnglish/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'U' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::French) {
        WWP_PATH = "/vol/content/UsFrench/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'U' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Portuguese) {
        WWP_PATH = "/vol/content/UsPortuguese/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'U' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Spanish) {
        WWP_PATH = "/vol/content/UsSpanish/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Dutch) {
        WWP_PATH = "/vol/content/EuDutch/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::English) {
        WWP_PATH = "/vol/content/EuEnglish/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::French) {
        WWP_PATH = "/vol/content/EuFrench/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::German) {
        WWP_PATH = "/vol/content/EuGerman/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Italian) {
        WWP_PATH = "/vol/content/EuItalian/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Portuguese) {
        WWP_PATH = "/vol/content/EuPortuguese/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Russian) {
        WWP_PATH = "/vol/content/EuRussian/olive/1stNUP.xml";
    } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Spanish) {
        WWP_PATH = "/vol/content/EuSpanish/olive/1stNUP.xml";
    }

    return 0;
}

int result = getWWPPath();

static std::optional<FSFileHandle> config_handle{};

DECL_FUNCTION(FSStatus, FSOpenFile_VINO, FSClient *client, FSCmdBlock *block,
              const char *path, const char *mode, FSFileHandle *handle,
              FSErrorFlag errorMask) {

    // DEBUG("Wii U wants to open file: %s", path);

    if (config::connectToRose && strcmp(VINO_CONFIG_PATH, path) == 0) {
        FSStatus res = real_FSOpenFile_VINO(client, block, path, mode, handle, errorMask);
        config_handle = *handle;
        return res;
    }

    return real_FSOpenFile_VINO(client, block, path, mode, handle, errorMask);
}

DECL_FUNCTION(FSStatus, FSReadFile_VINO, FSClient *client, FSCmdBlock *block, uint8_t *buffer, uint32_t size, uint32_t count,
              FSFileHandle handle, uint32_t unk1, uint32_t flags) {
    if (size != 1) {
        DEBUG_FUNCTION_LINE("Falied to patch vino config. Size is not 1");
    }

    if (config_handle && *config_handle == handle) {
        DEBUG_FUNCTION_LINE("Trying to read vino config detected, returning patched data.");
        strlcpy((char *) buffer, (const char *) rose_config_txt, size * count);
        return (FSStatus) count;
    }

    return real_FSReadFile_VINO(client, block, buffer, size, count, handle, unk1, flags);
}

DECL_FUNCTION(FSStatus, FSCloseFile_VINO, FSClient *client, FSCmdBlock *block, FSFileHandle handle, FSErrorFlag errorMask) {
    if (handle == config_handle) {
        DEBUG("Closing Vino config file and resetting handle");
        config_handle.reset();
    }

    return real_FSCloseFile_VINO(client, block, handle, errorMask);
}

WUPS_MUST_REPLACE_FOR_PROCESS(FSOpenFile_VINO, WUPS_LOADER_LIBRARY_COREINIT, FSOpenFile, WUPS_FP_TARGET_PROCESS_TVII);
WUPS_MUST_REPLACE_FOR_PROCESS(FSReadFile_VINO, WUPS_LOADER_LIBRARY_COREINIT, FSReadFile, WUPS_FP_TARGET_PROCESS_TVII);
WUPS_MUST_REPLACE_FOR_PROCESS(FSCloseFile_VINO, WUPS_LOADER_LIBRARY_COREINIT, FSCloseFile, WUPS_FP_TARGET_PROCESS_TVII);

DECL_FUNCTION(FSStatus, FSOpenFile_WWP, FSClient *client, FSCmdBlock *block,
              const char *path, const char *mode, FSFileHandle *handle,
              FSErrorFlag errorMask) {

    // DEBUG("Wii U wants to open file: %s", path);

    if (config::patchWWP && strcmp(WWP_PATH, path) == 0) {
        FSStatus res = real_FSOpenFile_WWP(client, block, path, mode, handle, errorMask);
        config_handle = *handle;
        return res;
    }

    return real_FSOpenFile_WWP(client, block, path, mode, handle, errorMask);
}

DECL_FUNCTION(FSStatus, FSReadFile_WWP, FSClient *client, FSCmdBlock *block, uint8_t *buffer, uint32_t size, uint32_t count,
              FSFileHandle handle, uint32_t unk1, uint32_t flags) {
    if (size != 1) {
        DEBUG_FUNCTION_LINE("Falied to patch 1stNUP. Size is not 1");
    }


    if (config_handle && *config_handle == handle) {
        DEBUG_FUNCTION_LINE("Trying to read 1stNUP detected, returning patched data.");
        if (utils::getConsoleRegion() == 'U' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::English) {
            strlcpy((char *) buffer, (const char *) UsEnglish_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'U' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::French) {
            strlcpy((char *) buffer, (const char *) UsFrench_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'U' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Portuguese) {
            strlcpy((char *) buffer, (const char *) UsPortuguese_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'U' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Spanish) {
            strlcpy((char *) buffer, (const char *) UsSpanish_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Dutch) {
            strlcpy((char *) buffer, (const char *) EuDutch_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::English) {
            strlcpy((char *) buffer, (const char *) EuEnglish_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::French) {
            strlcpy((char *) buffer, (const char *) EuFrench_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::German) {
            strlcpy((char *) buffer, (const char *) EuGerman_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Italian) {
            strlcpy((char *) buffer, (const char *) EuItalian_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Portuguese) {
            strlcpy((char *) buffer, (const char *) EuPortuguese_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Russian) {
            strlcpy((char *) buffer, (const char *) EuRussian_xml, size * count);
            return (FSStatus) count;
        } else if (utils::getConsoleRegion() == 'E' && utils::getConsoleLanguage() == nn::swkbd::LanguageType::Spanish) {
            strlcpy((char *) buffer, (const char *) EuSpanish_xml, size * count);
            return (FSStatus) count;
        }
    }

    return real_FSReadFile_WWP(client, block, buffer, size, count, handle, unk1, flags);
}

DECL_FUNCTION(FSStatus, FSCloseFile_WWP, FSClient *client, FSCmdBlock *block, FSFileHandle handle, FSErrorFlag errorMask) {
    if (handle == config_handle) {
        DEBUG("Closing 1stNUP file and resetting handle");
        config_handle.reset();
    }

    return real_FSCloseFile_WWP(client, block, handle, errorMask);
}

WUPS_MUST_REPLACE_FOR_PROCESS(FSOpenFile_WWP, WUPS_LOADER_LIBRARY_COREINIT, FSOpenFile, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(FSReadFile_WWP, WUPS_LOADER_LIBRARY_COREINIT, FSReadFile, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(FSCloseFile_WWP, WUPS_LOADER_LIBRARY_COREINIT, FSCloseFile, WUPS_FP_TARGET_PROCESS_WII_U_MENU);