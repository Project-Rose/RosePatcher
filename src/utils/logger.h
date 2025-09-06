// Taken from the Inkay plugin by Pretendo Network. Original code credit goes to Pretendo.
// Edited by Cooper (@coopeeo) 

#pragma once

#include <string.h>
#include <whb/log.h>
#include <whb/log_module.h>
#include <whb/log_udp.h>
#include <whb/log_cafe.h>

#define __FILENAME_X__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILENAME_X__)

#define OSFATAL_FUNCTION_LINE(FMT, ARGS...)do { \
    OSFatal_printf("Rosé Patcher - OSFatal - [%s]%s@L%04d: " FMT "",__FILENAME__,__FUNCTION__, __LINE__, ## ARGS); \
    } while (0)

#define DEBUG_FUNCTION_LINE(FMT, ARGS...)do { \
    WHBLogPrintf("Rosé Patcher - Debug - [%s]%s@L%04d: " FMT "",__FILENAME__,__FUNCTION__, __LINE__, ## ARGS); \
    } while (0);

#define DEBUG_FUNCTION_LINE_WRITE(FMT, ARGS...)do { \
    WHBLogWritef("Rosé Patcher - Debug - [%s]%s@L%04d: " FMT "",__FILENAME__,__FUNCTION__, __LINE__, ## ARGS); \
    } while (0);

#define DEBUG(FMT, ARGS...)do { \
    WHBLogPrintf("Rosé Patcher - Debug: " FMT "", ## ARGS); \
    } while (0);

#define DEBUG_WRITE(FMT, ARGS...)do { \
    WHBLogWritef("Rosé Patcher - Debug: " FMT "", ## ARGS); \
    } while (0);
