#include <wups.h>

#include <coreinit/dynload.h>
#include <coreinit/filesystem.h>

#include <cstring>
#include <string>
#include <vector>

#include "olvFix.hpp"
#include "../config.hpp"
#include "../utils/patch.hpp"
#include "../utils/logger.h"

namespace patches::olv {
    const char original_discovery_url[] = "discovery.olv.nintendo.net/v1/endpoint";
    const char new_discovery_url[]      = "discovery.olv.pretendo.cc/v1/endpoint";

    void osdynload_notify_callback(OSDynLoad_Module module, void *ctx,
                                   OSDynLoad_NotifyReason reason, OSDynLoad_NotifyData *rpl) {
        if (reason == OS_DYNLOAD_NOTIFY_LOADED) {
            if (!rpl->name)
                return;
            
            if (!std::string_view(rpl->name).ends_with("nn_olv.rpl") || !config::connectToRose)
                return;


            utils::patch::replace_mem(rpl->dataAddr, rpl->dataSize, original_discovery_url,
                        sizeof(original_discovery_url), new_discovery_url, sizeof(new_discovery_url));
        }
    }

    DECL_FUNCTION(int, FSOpenFile, FSClient *pClient, FSCmdBlock *pCmd,
                  const char *path, const char *mode, int *handle, int error) {
        if (strcmp("/vol/content/vino_config.txt", path) == 0) {
            OSDynLoad_AddNotifyCallback(&osdynload_notify_callback, nullptr);
        }

        return real_FSOpenFile(pClient, pCmd, path, mode, handle, error);;
    }

    WUPS_MUST_REPLACE_FOR_PROCESS(FSOpenFile, WUPS_LOADER_LIBRARY_COREINIT, FSOpenFile, WUPS_FP_TARGET_PROCESS_TVII);
} // namespace olv
