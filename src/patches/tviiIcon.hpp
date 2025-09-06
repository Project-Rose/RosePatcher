#pragma once
#include <wups.h>
#include <coreinit/dynload.h>

#include <cstring>
#include <string>
#include <vector>

#include "../utils/patch.hpp"

namespace patches::icon {
    extern const char root_rpx_check[];

    void perform_men_patches(bool enable);

    void perform_hbm_patches(bool enable);
}; // namespace icon