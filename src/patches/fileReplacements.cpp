#include <wups.h>
#include <coreinit/filesystem.h>

#include "../config.hpp"
#include "../utils/Notification.hpp"
#include "../utils/logger.h"
#include "../utils/patch.hpp"

#include "rose_config_txt.h" // included at runtime

#define VINO_CONFIG_PATH "/vol/content/vino_config.txt"

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
