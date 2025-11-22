#include <wups.h>
#include <coreinit/dynload.h>
#include <nsysnet/nssl.h>

#include "ssl.hpp"

#include "../utils/Notification.hpp"
#include "../utils/logger.h"
#include "../utils/patch.hpp"
#include "../config.hpp"

#include "gts_der.h" // Included at runtime

namespace patches::ssl {
    void addCertificateToWebKit() {
        if(config::certificateAdded) {
            return;
        }

        OSDynLoad_Module libwkc = 0;
        void* (*WKC_SSLRegisterRootCAByDER)(const char* cert, int cert_len) = nullptr;

        if (!config::connectToRose) {
            return;
        }

        OSDynLoad_Error ret = OSDynLoad_IsModuleLoaded("libwkc", &libwkc);

        if (ret != OS_DYNLOAD_OK || libwkc == 0) {
            DEBUG_FUNCTION_LINE("OSDynLoad_Acquire failed to get libwkc.rpl in tvii. (Most likely not loaded). With error: %d", ret);
            return;
        }

        if (OSDynLoad_FindExport(libwkc, OS_DYNLOAD_EXPORT_FUNC, "WKCWebKitSSLRegisterRootCAByDER__3WKCFPCci", (void **) &WKC_SSLRegisterRootCAByDER) != OS_DYNLOAD_OK) {
            DEBUG_FUNCTION_LINE("FindExport setDeveloperExtrasEnabled__Q2_3WKC11WKCSettingsFb failed.");
            return;
        }

        WKC_SSLRegisterRootCAByDER((const char *)(gts_der), gts_der_size);
        config::certificateAdded = true;
    }
}; // namespace ssl

DECL_FUNCTION(NSSLError, NSSLAddServerPKI, NSSLContextHandle context, NSSLServerCertId pki) {
    if (config::connectToRose && !config::gtsAdded) {
        NSSLAddServerPKIExternal(context, gts_der, gts_der_size, 0);
        // DEBUG_FUNCTION_LINE("Added GTS certificate to NSSL context. code: %d", ret);
        config::gtsAdded = true;
    }

    return real_NSSLAddServerPKI(context, pki);
}

WUPS_MUST_REPLACE_FOR_PROCESS(NSSLAddServerPKI, WUPS_LOADER_LIBRARY_NSYSNET, NSSLAddServerPKI, WUPS_FP_TARGET_PROCESS_TVII);