#pragma once
#include <coreinit/dynload.h>

#include "../utils/logger.h"

namespace patches::ssl {
    void addCertificateToWebKit();
}; // namespace ssl