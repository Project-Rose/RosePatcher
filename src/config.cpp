#include <algorithm>
#include <format>
#include <wups.h>
#include <wups/config.h>
#include <wups/config/WUPSConfigCategory.h>
#include <wups/config/WUPSConfigItem.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemStub.h>
#include <wups/storage.h>
#include <coreinit/bsp.h>
#include <coreinit/launch.h>
#include <coreinit/mcp.h>
#include <coreinit/title.h>
#include <nn/act/client_cpp.h>
#include <sysapp/launch.h>
#include <sysapp/title.h>

#include "config.hpp"
#include "utils/utils.hpp"
#include "utils/logger.h"

extern "C" MCPError MCP_GetDeviceId(int handle, char* out);
extern "C" MCPError MCP_GetCompatDeviceId(int handle, char* out);

namespace config {
  std::string replacementToken;
  bool connectToRose = CONNECT_TO_ROSE_DEFUALT_VALUE;
  bool tviiIconHBM = TVII_ICON_HBM_PATCH_DEFAULT_VALUE;
  bool tviiIconWUM = TVII_ICON_WUM_PATCH_DEFAULT_VALUE;
  bool forceJPNconsole = FORCE_JPN_CONSOLE_DEFAULT_VALUE;
  bool needRelaunch = false;
  bool certificateAdded = false;
  bool gtsAdded = false;
  bool enableRemindPoll = true;

  // Connect to Rose setting event
  void connectToRoseChanged(ConfigItemBoolean *item, bool newValue) {
    if (newValue != connectToRose) {
      WUPSStorageAPI::Store(CONNECT_TO_ROSE_CONFIG_ID, newValue);
    }

    connectToRose = newValue;
  }

  void tviiIconHBMChanged(ConfigItemBoolean *item, bool newValue) {
    if (tviiIconHBM != newValue) {
      WUPSStorageAPI::Store(TVII_ICON_HBM_PATCH_COFNIG_ID, newValue);
    }

    tviiIconHBM = newValue;
  }

  void tviiIconWUMChanged(ConfigItemBoolean *item, bool newValue) {
    if (tviiIconWUM != newValue) {
      WUPSStorageAPI::Store(TVII_ICON_WUM_PATCH_COFNIG_ID, newValue);
    }

    tviiIconWUM = newValue;
    auto title = OSGetTitleID();

    // https://github.com/PretendoNetwork/Inkay/blob/8da5fde9621d6b5d60adfa8af62d86d30a33882b/plugin/src/config.cpp#L123C1-L123C88
    uint64_t wiiu_menu_tid = _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_WII_U_MENU);
    if (title == wiiu_menu_tid) {
      needRelaunch = true;
    }
  }

  // Open event for the Aroma config menu
  WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle) {
    WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

    try {
      // Add setting items
      root.add(WUPSConfigItemStub::Create("-- General --"));
      root.add(WUPSConfigItemBoolean::Create(CONNECT_TO_ROSE_CONFIG_ID, "Connect to Rosé", CONNECT_TO_ROSE_DEFUALT_VALUE, connectToRose, connectToRoseChanged));
      
      if (!utils::isJapanConsole()) {
        root.add(WUPSConfigItemStub::Create("-- TVii Icons --"));
        root.add(WUPSConfigItemBoolean::Create(TVII_ICON_HBM_PATCH_COFNIG_ID, "Add TVii Icon to the \ue073 Menu", TVII_ICON_HBM_PATCH_DEFAULT_VALUE, tviiIconHBM, tviiIconHBMChanged));
        root.add(WUPSConfigItemBoolean::Create(TVII_ICON_WUM_PATCH_COFNIG_ID, "Add TVii Icon to the Wii U Menu", TVII_ICON_WUM_PATCH_DEFAULT_VALUE, tviiIconWUM, tviiIconWUMChanged));
        root.add(WUPSConfigItemStub::Create("Note: Wii U Menu will restart if \"Add TVii Icon to the Wii U Menu\""));
        root.add(WUPSConfigItemStub::Create("is toggled."));
      }
    } catch (std::exception &e) {
      DEBUG_FUNCTION_LINE("Creating config menu failed: %s", e.what());
      return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }

    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
  }

  // Close event for the Aroma config menu
  void ConfigMenuClosedCallback() {
    WUPSStorageAPI::SaveStorage();
    if (config::needRelaunch) {
      OSForceFullRelaunch();
      SYSLaunchMenu();
      config::needRelaunch = false;
    }
  }

  // Config stuff in plugin initialization
  void InitializeConfig() {
      // Add the config
    WUPSConfigAPIOptionsV1 configOptions = {.name = "Rosé Patcher"};
    if (WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback,
                          ConfigMenuClosedCallback) !=
        WUPSCONFIG_API_RESULT_SUCCESS) {
      DEBUG_FUNCTION_LINE("Failed to init config api");
    }

    // Add get saved values
    WUPSStorageError storageRes;
    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(CONNECT_TO_ROSE_CONFIG_ID, connectToRose, CONNECT_TO_ROSE_DEFUALT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
      DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)", WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
    }
    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(TVII_ICON_HBM_PATCH_COFNIG_ID, tviiIconHBM, TVII_ICON_HBM_PATCH_DEFAULT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
      DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)", WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
    }
    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(TVII_ICON_WUM_PATCH_COFNIG_ID, tviiIconWUM, TVII_ICON_WUM_PATCH_DEFAULT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
      DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)", WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
    }
    
    // For when we can't detect someones console region and their console region is actually Japan
    if ((storageRes = WUPSStorageAPI::GetOrStoreDefault(FORCE_JPN_CONSOLE_CONFIG_ID, forceJPNconsole, FORCE_JPN_CONSOLE_DEFAULT_VALUE)) != WUPS_STORAGE_ERROR_SUCCESS) {
      DEBUG_FUNCTION_LINE("GetOrStoreDefault failed: %s (%d)", WUPSStorageAPI_GetStatusStr(storageRes), storageRes);
    }

    int handle = MCP_Open();
    MCPSysProdSettings settings alignas(0x40);
    MCPError error = MCP_GetSysProdSettings(handle, &settings);
    if(error) {
      replacementToken = "";
      DEBUG_FUNCTION_LINE("MCP_GetSysProdSettings failed");
    }

    char key[10];

    // based on various sources
    // from https://github.com/RiiConnect24/UTag/blob/2287ef6c21e18de77162360cca53c1ccb1b30759/src/main.cpp#L26
    std::string filePath = std::format("fs:/vol/external01/wiiu/rose_key_{:d}.txt", nn::act::GetPrincipalId());
    FILE *fp = fopen(filePath.c_str(), "r");
    if (!fp) {
      DEBUG_FUNCTION_LINE("File %s found, generating a default.", filePath.c_str());
      fclose(fp);
      fp = fopen(filePath.c_str(), "w");

      // enable RNG
      srand(time(NULL));

      std::string newKey = "";

      // open disclosure: made w/ help of chatgpt
      for(int i = 0; i < 10; i++) {
        int randNum = rand() % 62;
        if (randNum < 26) {
          newKey += 'a' + randNum;  // lowercase letters a-z
        } else if (randNum < 52) {
          newKey += 'A' + (randNum - 26);  // uppercase letters A-Z
        } else {
          newKey += '0' + (randNum - 52);  // digits 0-9
        }
      }

      DEBUG_FUNCTION_LINE("%s", newKey.c_str());
      fputs(newKey.c_str(), fp);
      strcpy(key, newKey.c_str());

      fclose(fp);
    } else {
      fread(key, 10, 1, fp);
      DEBUG_FUNCTION_LINE("key: %s", key);

      fclose(fp);
    }

    MCP_Close(handle);
    replacementToken = std::format("[{}, {}, {}]", key, settings.code_id, settings.serial_id);
    DEBUG_FUNCTION_LINE("Replacement token: %s", replacementToken.c_str());
    reverse(std::next(replacementToken.begin()), std::prev(replacementToken.end()));
  }

} // namespace config
