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

#include "logger.h"

namespace utils::patch {
    OSDynLoad_NotifyData men_rpx;
    OSDynLoad_NotifyData hbm_rpx;

    uint32_t find_mem(uint32_t start, uint32_t size, const char *original_val,
                        size_t original_val_sz) {
        for (uint32_t addr = start; addr < start + size - original_val_sz; addr++) {
            if (memcmp(original_val, (void *) addr, original_val_sz) == 0) {
                return addr;
            }
        }
        return 0;
    }

    bool replace_mem(uint32_t start, uint32_t size, const char *original_val,
                        size_t original_val_sz, const char *new_val, size_t new_val_sz) {
        uint32_t addr = find_mem(start, size, original_val, original_val_sz);
        if (addr) {
            DEBUG_FUNCTION_LINE("replace: writing to %08X (%s) with %s", addr,
                                original_val, new_val);
            KernelCopyData(OSEffectiveToPhysical(addr),
                        OSEffectiveToPhysical((uint32_t) new_val), new_val_sz);
            return true;
        }
        return false;
    }

    bool patch_instruction(void *instr, uint32_t original, uint32_t replacement) {
        uint32_t current = *(uint32_t *) instr;

        if (current != original)
            return current == replacement;

        DEBUG_FUNCTION_LINE("patch_instruction: writing to %08X (%08X) with %08X",
                            (uint32_t) instr, current, replacement);

        KernelCopyData(OSEffectiveToPhysical((uint32_t) instr),
                    OSEffectiveToPhysical((uint32_t) &replacement),
                    sizeof(replacement));
        DCFlushRange(instr, 4);
        ICInvalidateRange(instr, 4);

        current = *(uint32_t *) instr;

        return true;
    }

    bool patch_dynload_instructions() {
        uint32_t *patch1 = ((uint32_t *) &OSDynLoad_GetNumberOfRPLs) + 6;
        uint32_t *patch2 = ((uint32_t *) &OSDynLoad_GetRPLInfo) + 22;

        if (!patch_instruction(patch1, 0x41820038 /* beq +38 */,
                            0x60000000 /* nop */))
            return false;
        if (!patch_instruction(patch2, 0x41820100 /* beq +100 */,
                            0x60000000 /* nop */))
            return false;

        return true;
    }

    bool get_rpl_info(std::vector<OSDynLoad_NotifyData> &rpls) {
        int num_rpls = OSDynLoad_GetNumberOfRPLs();

        DEBUG_FUNCTION_LINE("get_rpl_info: %d RPL(s) running", num_rpls);

        if (num_rpls == 0) {
            return false;
        }

        rpls.resize(num_rpls);

        bool ret = OSDynLoad_GetRPLInfo(0, num_rpls, rpls.data());

        return ret;
    }

    bool find_rpl(OSDynLoad_NotifyData &found_rpl, const std::string &name) {
        if (!patch_dynload_instructions()) {
            DEBUG_FUNCTION_LINE("find_rpl: failed to patch dynload functions");
            return false;
        }

        std::vector<OSDynLoad_NotifyData> rpl_info;
        if (!get_rpl_info(rpl_info)) {
            DEBUG_FUNCTION_LINE("find_rpl: failed to get rpl info");
            return false;
        }

        DEBUG_FUNCTION_LINE("find_rpl: got rpl info");

        for (const auto &rpl : rpl_info) {
            if (rpl.name == nullptr || rpl.name[0] == '\0') {
                continue;
            }
            if (std::string_view(rpl.name).ends_with(name)) {
                found_rpl = rpl;
                DEBUG_FUNCTION_LINE("find_rpl: found rpl %s", name.c_str());
                return true;
            }
        }

        return false;
    }
    
}; // namespace patch