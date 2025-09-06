#include <wups.h>

#include <kernel/kernel.h>

#include <coreinit/cache.h>
#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <coreinit/filesystem.h>
#include <coreinit/memorymap.h>

#include <cstring>
#include <string>
#include <vector>

#include "../config.hpp"
#include "../utils/patch.hpp"
#include "../utils/logger.h"

namespace patches::icon {

    const char root_rpx_check[] = "/vol/external01/wiiu/payload.elf";

    OSDynLoad_NotifyData men_rpx;
    OSDynLoad_NotifyData hbm_rpx;

    void perform_men_patches(bool enable) {
        if (utils::isJapanConsole()) {
            DEBUG_FUNCTION_LINE("Japan console detected, skipping men patches");
            return;
        }

        if (!utils::is555OrHigher()) {
            DEBUG_FUNCTION_LINE("Console not version 5.5.5 or greater, skipping men patches");
            return;
        }

        if (!utils::patch::find_rpl(men_rpx, "men.rpx")) {
            DEBUG_FUNCTION_LINE("perform_men_patches: couldnt find men.rpx");
            return;
        }

        if (utils::patch::find_mem(men_rpx.dataAddr, men_rpx.dataSize, root_rpx_check, sizeof(root_rpx_check))) {
            DEBUG_FUNCTION_LINE("perform_men_patches: men.rpx has been replaced by root.rpx, skipping patches ...");
            return;
        }

        if (enable) {
            DEBUG_FUNCTION_LINE("perform_men_patches: enabling tvii patches");
            utils::patch::patch_instruction((uint8_t *) men_rpx.textAddr + 0x1e0b10, 0x5403d97e,
                              0x38600001); // v277
            utils::patch::patch_instruction((uint8_t *) men_rpx.textAddr + 0x1e0a20, 0x5403d97e,
                              0x38600001); // v257
        } else {
            DEBUG_FUNCTION_LINE("perform_men_patches: disabling tvii patches");
            utils::patch::patch_instruction((uint8_t *) men_rpx.textAddr + 0x1e0b10, 0x38600001,
                              0x5403d97e); // v277
            utils::patch::patch_instruction((uint8_t *) men_rpx.textAddr + 0x1e0a20, 0x38600001,
                              0x5403d97e); // v257
        }
    }

    void perform_hbm_patches(bool enable) {
        WHBLogUdpInit();
        WHBLogCafeInit();
        WHBLogModuleInit();

        if (utils::isJapanConsole()) {
            DEBUG_FUNCTION_LINE("Japan console detected, skipping hbm patches");
            return;
        }

        if (!utils::is555OrHigher()) {
            DEBUG_FUNCTION_LINE("Console not version 5.5.5 or greater, skipping hbm patches");
            return;
        }

        if (!utils::patch::find_rpl(hbm_rpx, "hbm.rpx")) {
            DEBUG_FUNCTION_LINE("perform_hbm_patches: couldnt find hbm.rpx");
            return;
        }

        if (enable) {
            DEBUG_FUNCTION_LINE("perform_hbm_patches: enabling tvii patches");
            utils::patch::patch_instruction((uint8_t *) hbm_rpx.textAddr + 0x0ec430, 0x5403d97e,
                              0x38600001); // v197
            utils::patch::patch_instruction((uint8_t *) hbm_rpx.textAddr + 0x0ec434, 0x7c606110,
                              0x38600001); // v180
        } else {
            DEBUG_FUNCTION_LINE("perform_hbm_patches: disabling tvii patches");
            utils::patch::patch_instruction((uint8_t *) hbm_rpx.textAddr + 0x0ec430, 0x38600001,
                              0x5403d97e); // v197
            utils::patch::patch_instruction((uint8_t *) hbm_rpx.textAddr + 0x0ec434, 0x38600001,
                              0x7c606110); // v180
        }
    }
}; // namespace icon

DECL_FUNCTION(int, FSOpenFile_TVII, FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error) {
    if (strcmp("/vol/content/Common/Package/Hbm2-2.pack", path) == 0) {
        patches::icon::perform_hbm_patches(config::tviiIconHBM);
    }
    
    return real_FSOpenFile_TVII(pClient, pCmd, path, mode, handle, error);;
}

WUPS_MUST_REPLACE_FOR_PROCESS(FSOpenFile_TVII, WUPS_LOADER_LIBRARY_COREINIT, FSOpenFile, WUPS_FP_TARGET_PROCESS_ALL);