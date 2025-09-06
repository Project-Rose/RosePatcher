#pragma once
#include <coreinit/dynload.h>

#include "../utils/logger.h"

namespace patches::olv {
    extern const char original_discovery_url[];
    extern const char new_discovery_url[];

    void osdynload_notify_callback(OSDynLoad_Module module, void *ctx,
                                   OSDynLoad_NotifyReason reason, OSDynLoad_NotifyData *rpl);
} // namespace olv