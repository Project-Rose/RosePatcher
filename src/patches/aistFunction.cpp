#include <wups.h>

#include <coreinit/dynload.h>
#include <coreinit/filesystem.h>
#include <coreinit/memorymap.h>
#include <coreinit/title.h>
#include <nsysnet/nssl.h>

#include <function_patcher/function_patching.h>
#include <notifications/notifications.h>

#include "../config.hpp"
#include "../utils/Notification.hpp"
#include "../utils/logger.h"
#include "../utils/patch.hpp"
#include "../utils/token.hpp"
#include "../utils/utils.hpp"
#include "ssl.hpp"

PatchedFunctionHandle AISTpatchHandleBetter = 0;
int AISTCallCount = 0;

DECL_FUNCTION(int, AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4,
              uint8_t *token, const char *client_id) {
  if (client_id && utils::isVinoClientID(client_id) && config::connectToRose) {
    AISTCallCount++;
    DEBUG_FUNCTION_LINE("AISTCallCount is %d!", AISTCallCount);
    patches::ssl::addCertificateToWebKit();
    DEBUG_FUNCTION_LINE("Faking service sucess for '%s' (should be Vino)",
                        client_id);
    DEBUG_FUNCTION_LINE("Size: %d", token::currentReplacementToken.size());
    memcpy(token, token::currentReplacementToken.c_str(),
           token::currentReplacementToken.size());
    return 0;
  }

  DEBUG_FUNCTION_LINE("Not faking - calling real");
  // make sure we set the token again in case the following replaces it just in
  // case
  return real_AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4(token,
                                                                    client_id);
}

DECL_FUNCTION(void, NSSLInit) {
  OSDynLoad_Module NN_ACT = 0;
  uint32_t AISTaddressVIR = 0;
  uint32_t AISTaddress = 0;
  bool isAlreadyPatched = false;
  PatchedFunctionHandle AISTpatchHandle = 0;
  bool AISTpatchSuccess = false;

  // Call the original function
  real_NSSLInit();

  // Init Logging Functions
  WHBLogModuleInit();
  WHBLogUdpInit();
  WHBLogCafeInit();
  // token::updCurrentToken();

  // Notify about the patch
  DEBUG("Rosé Patcher: Trying to patch AcquireIndependentServiceToken via "
        "NSSLInit\n");

  FunctionPatcher_IsFunctionPatched(AISTpatchHandleBetter, &isAlreadyPatched);

  DEBUG_FUNCTION_LINE("AISTCallCount is %d!", AISTCallCount);

  if (AISTCallCount >= 3) {
    DEBUG_FUNCTION_LINE("AISTCallCount is %d, removing patch...",
                        AISTCallCount);
    AISTCallCount = 0;
    FunctionPatcher_RemoveFunctionPatch(AISTpatchHandleBetter);
  } else if (isAlreadyPatched == true) {
    DEBUG_FUNCTION_LINE(
        "AISTCallCount is %d, but we got true of isalreadypatched so return.",
        AISTCallCount);
    AISTCallCount = 0;
    FunctionPatcher_RemoveFunctionPatch(AISTpatchHandleBetter);
    // return;
  }

  if (!config::connectToRose) {
    DEBUG_FUNCTION_LINE("\"Connect to Rosé\" patch is disabled, skipping...");
    return;
  }

  // Acquire the nn_act module
  if (OSDynLoad_Acquire("nn_act.rpl", &NN_ACT) != OS_DYNLOAD_OK) {
    DEBUG_FUNCTION_LINE("Failed to acquire nn_act.rpl module.");
    return;
  }

  // Find the AcquireIndependentServiceToken function addresses
  if (OSDynLoad_FindExport(
          NN_ACT, OS_DYNLOAD_EXPORT_FUNC,
          "AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4",
          (void **)&AISTaddressVIR) !=
      OS_DYNLOAD_OK) { // Get the physical address
    DEBUG_FUNCTION_LINE(
        "Failed to find AcquireIndependentServiceToken function in nn_act.rpl");
    OSDynLoad_Release(NN_ACT);
    return;
  }
  AISTaddress =
      OSEffectiveToPhysical(AISTaddressVIR); // Get the virtual address

  // Show results of the AcquireIndependentServiceToken function address
  // findings DEBUG("AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4
  // results\n" "Rosé Patcher:   Physical Address: %d\n" "Rosé Patcher: Virtual
  // Address: %d\n" "Rosé Patcher: -- END OF AcquireIndependentServiceToken
  // ADDRESS RESULTS --", &AISTaddress, &AISTaddressVIR);

  // Make function replacement data
  function_replacement_data_t AISTpatch =
      REPLACE_FUNCTION_VIA_ADDRESS_FOR_PROCESS(
          AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4, AISTaddress,
          AISTaddressVIR, FP_TARGET_PROCESS_TVII);

  // Patch the function
  if (FunctionPatcher_AddFunctionPatch(&AISTpatch, &AISTpatchHandle,
                                       &AISTpatchSuccess) !=
      FunctionPatcherStatus::FUNCTION_PATCHER_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("Failed to add patch.");
    OSDynLoad_Release(NN_ACT);
    return;
  }

  AISTpatchHandleBetter = AISTpatchHandle;

  if (AISTpatchSuccess == false) {
    DEBUG_FUNCTION_LINE("Failed to add patch.");
    OSDynLoad_Release(NN_ACT);
    return;
  }

  // Notify about the patch success
  DEBUG("Patched AcquireIndependentServiceToken via NSSLInit");
  OSDynLoad_Release(NN_ACT);
}

WUPS_MUST_REPLACE_FOR_PROCESS(NSSLInit, WUPS_LOADER_LIBRARY_NSYSNET, NSSLInit,
                              WUPS_FP_TARGET_PROCESS_TVII);
