#include <wups.h>
#include <coreinit/dynload.h>
#include <nsysnet/nssl.h>

#include "../utils/Notification.hpp"
#include "../utils/logger.h"
#include "../utils/patch.hpp"
#include "../config.hpp"

#include "gts_der.h" // Included at runtime

DECL_FUNCTION(NSSLError, NSSLAddServerPKI, NSSLContextHandle context, NSSLServerCertId pki) {
    if (config::connectToRose && !config::gtsAdded) {
        NSSLAddServerPKIExternal(context, gts_der, gts_der_size, 0);
        // DEBUG_FUNCTION_LINE("Added GTS certificate to NSSL context. code: %d", ret);
        config::gtsAdded = true;
    }

    return real_NSSLAddServerPKI(context, pki);
}

WUPS_MUST_REPLACE_FOR_PROCESS(NSSLAddServerPKI, WUPS_LOADER_LIBRARY_NSYSNET, NSSLAddServerPKI, WUPS_FP_TARGET_PROCESS_TVII);