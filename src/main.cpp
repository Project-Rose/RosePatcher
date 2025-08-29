#include <wups.h>
#include <wups/config_api.h>

#include <coreinit/title.h>
#include <curl/curl.h>
#include <function_patcher/function_patching.h>
#include <nn/ac.h>
#include <nn/act/client_cpp.h>
#include <nn/result.h>
#include <notifications/notifications.h>

#include "config.hpp"
#include "patches/tviiIcon.hpp"
#include "reminderpoller.hpp"
#include "utils/Notification.hpp"
#include "utils/logger.h"
#include "utils/token.hpp"

WUPS_PLUGIN_NAME("Rosé Patcher");
WUPS_PLUGIN_DESCRIPTION("Patcher for Project Rosé's Nintendo TVii revival service.");
WUPS_PLUGIN_VERSION("v1.2.3");
WUPS_PLUGIN_AUTHOR("Project Rosé Team");
WUPS_PLUGIN_LICENSE("GPLv2");

WUPS_USE_STORAGE("rosepatcher");
WUPS_USE_WUT_DEVOPTAB();

INITIALIZE_PLUGIN() {
  // Initialize libraries
  WHBLogModuleInit();
  WHBLogUdpInit();
  WHBLogCafeInit();
  nn::act::Initialize();
  FunctionPatcher_InitLibrary();

  curl_global_init(CURL_GLOBAL_DEFAULT);
  config::InitializeConfig();

  // Check if NotificationModule library is initialized
  if (NotificationModule_InitLibrary() != NOTIFICATION_MODULE_RESULT_SUCCESS) {
    DEBUG_FUNCTION_LINE("NotificationModule_InitLibrary failed");
  }

  if (config::connectToRose) {
    ShowNotification("Rosé patch enabled");
  } else {
    ShowNotification("Rosé patch disabled");
  }
}

DEINITIALIZE_PLUGIN() {
  curl_global_cleanup();
  patches::icon::perform_hbm_patches(false);

  nn::act::Finalize();
  WHBLogModuleDeinit();
  WHBLogUdpDeinit();
  WHBLogCafeDeinit();
  NotificationModule_DeInitLibrary();
  FunctionPatcher_DeInitLibrary();
}

ON_APPLICATION_START() {
  WHBLogModuleInit();
  WHBLogUdpInit();
  WHBLogCafeInit();

  nn::ac::Initialize();
  nn::ac::ConnectAsync();
  nn::act::Initialize();

  auto title = OSGetTitleID();
  if (config::tviiIconWUM) {
    if (title == 0x5001010040000 || title == 0x5001010040100 || title == 0x5001010040200) {
      patches::icon::perform_men_patches(true);
    }
  }

  if (config::enableRemindPoll) {
      reminderpoller::CreateReminderPoller();
  }
}

ON_APPLICATION_ENDS() {
  auto title = OSGetTitleID();
  if (title == 0x5001010040000 || title == 0x5001010040100 || title == 0x5001010040200) {
    patches::icon::perform_men_patches(false);
  }

  if (config::enableRemindPoll) {
    reminderpoller::should_kill = true;
  }
}

// ensure we update
DECL_FUNCTION(nn::Result, LoadConsoleAccount__Q2_2nn3actFUc13ACTLoadOptionPCcb, nn::act::SlotNo slot, nn::act::ACTLoadOption unk1, char const * unk2, bool unk3) {
  // we should load first
  nn::Result ret = real_LoadConsoleAccount__Q2_2nn3actFUc13ACTLoadOptionPCcb(slot, unk1, unk2, unk3);

  // Could be a new or removed user. TODO: isolate which function new users are called/deleted
  token::initToken();

  token::updCurrentToken();

  return ret;
}

WUPS_MUST_REPLACE(LoadConsoleAccount__Q2_2nn3actFUc13ACTLoadOptionPCcb, WUPS_LOADER_LIBRARY_NN_ACT, LoadConsoleAccount__Q2_2nn3actFUc13ACTLoadOptionPCcb);
