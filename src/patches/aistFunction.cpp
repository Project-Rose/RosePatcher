#include <wups.h>

#include <coreinit/dynload.h>
#include <coreinit/filesystem.h>
#include <coreinit/title.h>
#include <coreinit/memorymap.h>
#include <nsysnet/nssl.h>

#include <function_patcher/function_patching.h>
#include <notifications/notifications.h>

#include "../config.hpp"
#include "../utils/Notification.hpp"
#include "../utils/logger.h"
#include "../utils/utils.hpp"
#include "../utils/patch.hpp"
#include "../utils/token.hpp"

uint32_t AISTaddressVIR = 0;
uint32_t AISTaddress = 0;
PatchedFunctionHandle AISTpatchHandle = 0;

DECL_FUNCTION(int, AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4, uint8_t *token, const char *client_id)
{
    if (client_id && utils::isVinoClientID(client_id) && config::connectToRose)
    {
        DEBUG_FUNCTION_LINE("Faking service sucess for '%s' (should be Vino)", client_id);
        DEBUG_FUNCTION_LINE("Size: %d", token::currentReplacementToken.size());
        memcpy(token, token::currentReplacementToken.c_str(), token::currentReplacementToken.size());
        return 0;
    }

    DEBUG_FUNCTION_LINE("Not faking - calling real");
    // make sure we set the token again in case the following replaces it just in case
    return real_AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4(token, client_id);
}

DECL_FUNCTION(void, NSSLInit)
{
    if (!config::connectToRose)
    {
        DEBUG_FUNCTION_LINE("\"Connect to Rosé\" patch is disabled, skipping...");
        return real_NSSLInit();
    }

    OSDynLoad_Module NN_ACT = 0;

    DEBUG_FUNCTION_LINE("Removing previous AIST patch (handle %08X)", AISTpatchHandle);
    FunctionPatcher_RemoveFunctionPatch(AISTpatchHandle);
    AISTpatchHandle = 0;

    // Notify about the patch
    DEBUG_FUNCTION_LINE("Trying to patch AcquireIndependentServiceToken via NSSLInit\n");

    // Acquire the nn_act module
    if (OSDynLoad_Acquire("nn_act.rpl", &NN_ACT) != OS_DYNLOAD_OK)
    {
        DEBUG_FUNCTION_LINE("Failed to acquire nn_act.rpl module.");
        return real_NSSLInit();
    }

    // Find the AcquireIndependentServiceToken function addresses
    if (OSDynLoad_FindExport(NN_ACT, OS_DYNLOAD_EXPORT_FUNC, "AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4", (void **)&AISTaddressVIR) != OS_DYNLOAD_OK)
    { // Get the physical address
        DEBUG_FUNCTION_LINE("Failed to find AcquireIndependentServiceToken function in nn_act.rpl");
        return real_NSSLInit();
    }

    AISTaddress = OSEffectiveToPhysical(AISTaddressVIR); // Get the virtual address

    function_replacement_data_t AISTpatch = REPLACE_FUNCTION_VIA_ADDRESS_FOR_PROCESS(
        AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4,
        AISTaddress,
        AISTaddressVIR,
        FP_TARGET_PROCESS_TVII);

    // Patch the function
    if (FunctionPatcher_AddFunctionPatch(&AISTpatch, &AISTpatchHandle, nullptr) != FunctionPatcherStatus::FUNCTION_PATCHER_RESULT_SUCCESS)
    {
        DEBUG_FUNCTION_LINE("Failed to add patch.");
        return real_NSSLInit();
    }

    // Notify about the patch success
    DEBUG_FUNCTION_LINE("Patched AcquireIndependentServiceToken via NSSLInit");
    return real_NSSLInit();
}

WUPS_MUST_REPLACE_FOR_PROCESS(NSSLInit, WUPS_LOADER_LIBRARY_NSYSNET, NSSLInit, WUPS_FP_TARGET_PROCESS_TVII);
