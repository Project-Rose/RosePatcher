#pragma once
#include <wups.h>
#include "utils.hpp"
#include <coreinit/dynload.h>

namespace utils::patch {
        uint32_t find_mem(uint32_t start, uint32_t size, const char *original_val, size_t original_val_sz);
        bool replace_mem(uint32_t start, uint32_t size, const char *original_val, size_t original_val_sz, const char *new_val, size_t new_val_sz);

        bool patch_instruction(void *instr, uint32_t original, uint32_t replacement);
        bool patch_dynload_instructions();

        bool get_rpl_info(std::vector<OSDynLoad_NotifyData> &rpls);
        bool find_rpl(OSDynLoad_NotifyData &found_rpl, const std::string &name);
}; // namespace patch