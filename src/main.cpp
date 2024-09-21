// Nova is real I think
// Note: used some of Inkay, the Pretendo Network plugin source code as a base
// and to make use of the notfications. Credit to Pretendo for developing Inkay
#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <coreinit/filesystem.h>
#include <coreinit/memorymap.h>
#include <coreinit/title.h>
#include <sysapp/launch.h>
#include <sysapp/switch.h>
#include <vpad/input.h>

#include <function_patcher/function_patching.h>

#include <notifications/notifications.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemStub.h>
#include <wups/config_api.h>

#include "Notification.h"
#include "utils/logger.h"

WUPS_PLUGIN_NAME("Rose Patcher");
WUPS_PLUGIN_DESCRIPTION("TVii config patcher for Project Rose");
WUPS_PLUGIN_VERSION("v1.1");
WUPS_PLUGIN_AUTHOR("Glitchii and Fangal");
WUPS_PLUGIN_LICENSE("GPLv2");

WUPS_USE_STORAGE("rosepatcher");
WUPS_USE_WUT_DEVOPTAB();

#define TVii_TITLE_ID 0x000500301001310A
#define TVii_CLIENT_ID "87a44dad436407e4ec47ad42ed68bf8c"

#define VINO_CONFIG_PATH "/vol/content/vino_config.txt"
#define VINO_CONFIG_SD_PATH "/vol/external01/TVii/vino_config.txt"

#define CONNECT_TO_ROSE_CONFIG_ID "connect_to_rose"
#define CONNECT_TO_ROSE_DEFUALT_VALUE true

#define REPLACE_DLM_CONFIG_ID "replace_download_management"
#define REPLACE_DLM_DEFAULT_VALUE false

FSMountSource mSource;
char mPath[128] = "";

bool connectToRose = true;
bool replaceDownloadManagement = false;
bool needRelaunch = false;

void connectToRoseChanged(ConfigItemBoolean *item, bool newValue) {
  if (newValue != connectToRose) {
    WUPSStorageAPI::Store(CONNECT_TO_ROSE_CONFIG_ID, newValue);
  }

  connectToRose = newValue;
}

void replaceDownloadManagementChanged(ConfigItemBoolean *item, bool newValue) {
  if (replaceDownloadManagement != newValue) {
    WUPSStorageAPI::Store(REPLACE_DLM_CONFIG_ID, newValue);
  }

  replaceDownloadManagement = newValue;
  needRelaunch = true;
}

WUPSConfigAPICallbackStatus
ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
  WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

  try {
    root.add(WUPSConfigItemBoolean::Create(
        CONNECT_TO_ROSE_CONFIG_ID, "Config Patch Enabled",
        CONNECT_TO_ROSE_DEFUALT_VALUE, connectToRose, connectToRoseChanged));
    root.add(WUPSConfigItemBoolean::Create(
        REPLACE_DLM_CONFIG_ID, "Replace Download Management",
        REPLACE_DLM_DEFAULT_VALUE, replaceDownloadManagement,
        replaceDownloadManagementChanged));
  } catch (std::exception &e) {
    DEBUG_FUNCTION_LINE("Creating config menu failed: %s", e.what());
    return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
  }

  return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback() {
  WUPSStorageAPI::SaveStorage();
  if (needRelaunch) {
    _SYSLaunchTitleWithStdArgsInNoSplash(OSGetTitleID(), nullptr);
    needRelaunch = false;
  }
}

INITIALIZE_PLUGIN() {
  WHBLogUdpInit();
  WHBLogCafeInit();
  FunctionPatcher_InitLibrary();

  WUPSConfigAPIOptionsV1 configOptions = {.name = "Rose Patcher"};
  if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback,
                         ConfigMenuClosedCallback) !=
      WUPSCONFIG_API_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("Failed to init config api");
  }

  WUPSStorageError storageRes;
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           CONNECT_TO_ROSE_CONFIG_ID, connectToRose,
           CONNECT_TO_ROSE_DEFUALT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }
  if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(
           REPLACE_DLM_CONFIG_ID, replaceDownloadManagement,
           REPLACE_DLM_DEFAULT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
    DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)",
                        WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
  }

  if (NotificationModule_InitLibrary() != NOTIFICATION_MODULE_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("NotificationModule_InitLibrary failed :(");
  }

  if (connectToRose) {
    ShowNotification("Rosè patch enabled");
  } else {
    ShowNotification("Rosè patch disabled");
  }
}

DEINITIALIZE_PLUGIN() {
  WHBLogUdpDeinit();
  WHBLogCafeDeinit();
  NotificationModule_DeInitLibrary();
  FunctionPatcher_DeInitLibrary();
}

ON_APPLICATION_START() {
  WHBLogUdpInit();
  WHBLogCafeInit();

  DEBUG_FUNCTION_LINE("RosePatcher: hihi");
}

DECL_FUNCTION(int, AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4, uint8_t* token, const char* client_id)
{

    if (client_id && strcmp(client_id, TVii_CLIENT_ID) == 0) {
        WHBLogPrintf("ReTViive: Faking service sucess for '%s'", client_id);
        return 0;
    }

    return real_AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4(token, client_id);
}

DECL_FUNCTION(int32_t, _SYSSwitchTo, SysAppPFID pfid) {
  SysAppPFID uPfid = pfid;

  VPADStatus status;
  VPADReadError err;

  VPADRead(VPAD_CHAN_0, &status, 1, &err);

  if (pfid == SYSAPP_PFID_DOWNLOAD_MANAGEMENT) {
    if (replaceDownloadManagement) {
      if (!(status.hold & VPAD_BUTTON_ZL)) {
        uPfid = SYSAPP_PFID_TVII;
      }
    }
  }

  return real__SYSSwitchTo(uPfid);
}

DECL_FUNCTION(int32_t, _SYSSwitchToOverlayFromHBM, SysAppPFID pfid) {
  SysAppPFID uPfid = pfid;

  VPADStatus status;
  VPADReadError err;

  VPADRead(VPAD_CHAN_0, &status, 1, &err);

  if (pfid == SYSAPP_PFID_DOWNLOAD_MANAGEMENT) {
    if (replaceDownloadManagement) {
      if (!(status.hold & VPAD_BUTTON_ZL)) {
        uPfid = SYSAPP_PFID_TVII;
      }
    }
  }

  return real__SYSSwitchToOverlayFromHBM(uPfid);
}

DECL_FUNCTION(FSStatus, FSOpenFile, FSClient *client, FSCmdBlock *block,
              const char *path, const char *mode, FSFileHandle *handle,
              FSErrorFlag errorMask) {
  if (connectToRose) {
    if (strcmp(VINO_CONFIG_PATH, path) == 0) {
      // Applets need to mount the SD card to access files on it :P
      FSGetMountSource(client, block, FS_MOUNT_SOURCE_SD, &mSource,
                       FS_ERROR_FLAG_ALL);
      FSMount(client, block, &mSource, mPath, sizeof(mPath), FS_ERROR_FLAG_ALL);
      FSStatus res = real_FSOpenFile(client, block, VINO_CONFIG_SD_PATH, mode,
                                     handle, errorMask);
      if (res != FS_STATUS_OK) {
        OSFatal("--------------- Error ---------------\n\n"
                "Error loading vino_config.txt\n\nPlease check if this file is "
                "in the correct directory:\n"
                "sd:/TVii/vino_config.txt");
      }

      return res;
    }
  }

  return real_FSOpenFile(client, block, path, mode, handle, errorMask);
}

DECL_FUNCTION(void, FSInit_TVii)
{
    OSDynLoad_Module NN_ACT = 0;
    uint32_t AISTaddressVIR = 0;
    uint32_t AISTaddress = 0;


    // Call the original function
    real_FSInit_TVii();
    WHBLogPrintf("ReTViive: Trying to patch AcquireIndependentServiceToken via FSInit");
    OSReport("ReTViive: Trying to patch AcquireIndependentServiceToken via FSInit");

    // Acquire the nn_act module
    if(OSDynLoad_Acquire("nn_act.rpl", &NN_ACT) != OS_DYNLOAD_OK) {
        WHBLogPrintf("ReTViive: failed to acquire nn_act module");
        OSReport("ReTViive: failed to acquire nn_act module");
        return;
    }

    // Find the AcquireIndependentServiceToken function
    if(OSDynLoad_FindExport(NN_ACT, OS_DYNLOAD_EXPORT_FUNC, "AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4", (void**)&AISTaddressVIR) != OS_DYNLOAD_OK) {
        WHBLogPrintf("ReTViive: failed to find AcquireIndependentServiceToken function in nn_act");
        OSReport("ReTViive: failed to find AcquireIndependentServiceToken function in nn_act");
        return;
    }
    AISTaddress = OSEffectiveToPhysical(AISTaddressVIR);

    OSReport("ReTViive: AISTaddress: %d AISTaddressVIR: %d", &AISTaddress, &AISTaddressVIR);

    // Patch the function
    function_replacement_data_t AISTpatch = REPLACE_FUNCTION_VIA_ADDRESS_FOR_PROCESS(AcquireIndependentServiceToken__Q2_2nn3actFPcPCcUibT4, AISTaddress, AISTaddressVIR, FP_TARGET_PROCESS_TVII);
    PatchedFunctionHandle AISTpatchHandle = 0;
    bool AISTpatchSuccess = false;
    FunctionPatcher_AddFunctionPatch(&AISTpatch, &AISTpatchHandle, &AISTpatchSuccess);
    if (AISTpatchSuccess == false) {
        WHBLogPrintf("ReTViive: Failed to add patch.");
        OSReport("ReTViive: Failed to add patch.");
        return;
    }
    WHBLogPrintf("ReTViive: Patched AcquireIndependentServiceToken via FSInit");
    OSReport("ReTViive: Patched AcquireIndependentServiceToken via FSInit");

}

WUPS_MUST_REPLACE_FOR_PROCESS(FSInit_TVii, WUPS_LOADER_LIBRARY_COREINIT, FSInit, WUPS_FP_TARGET_PROCESS_TVII);


WUPS_MUST_REPLACE(_SYSSwitchTo, WUPS_LOADER_LIBRARY_SYSAPP, _SYSSwitchTo);
WUPS_MUST_REPLACE_PHYSICAL_FOR_PROCESS(_SYSSwitchToOverlayFromHBM, 0x2E47373C,
                                       0x0E47373C,
                                       WUPS_FP_TARGET_PROCESS_HOME_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(FSOpenFile, WUPS_LOADER_LIBRARY_COREINIT,
                              FSOpenFile, WUPS_FP_TARGET_PROCESS_TVII);
