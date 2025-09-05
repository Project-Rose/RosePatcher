//
// Created by Joshua Peisach on 7/27/25.
//

#include "reminderpoller.hpp"

#include "config.hpp"
#include "utils/logger.h"
#include "utils/token.hpp"
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <curl/curl.h>
#include <exception>
#include <nn/act.h>
#include <thread>

namespace reminderpoller {
bool running = false;
bool should_kill = false;

size_t callback(char *data, size_t size, size_t nmemb, void *userdata) {
  DEBUG_FUNCTION_LINE("%s", data);
  return size * nmemb;
}

void poll_thread() {
  CURL *curl = curl_easy_init();
  while (true) {
    if (curl) {
      OSTime time = OSGetTime();

      // TODO: URL at configure time
      curl_easy_setopt(curl, CURLOPT_URL,
                       "https://projectrose.cafe/placeholder/lmao");
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

      curl_slist *headers = NULL;
      std::string pidString =
          std::format("PID: {:d}", nn::act::GetPersistentId());
      headers = curl_slist_append(headers, pidString.c_str());
      std::string tokenString =
          std::format("Token: {:s}", token::currentReplacementToken);
      headers = curl_slist_append(headers, tokenString.c_str());
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

      CURLcode res = curl_easy_perform(curl);
      if (res != 0) {
        DEBUG_FUNCTION_LINE("Couldn't perform: %s", curl_easy_strerror(res));
      } else {
        DEBUG_FUNCTION_LINE("Performed");
      }
      curl_slist_free_all(headers);
    }
    std::this_thread::sleep_for(std::chrono::seconds(60));
    if (should_kill) {
      break;
    }
  }
}

void CreateReminderPoller() {
  try {
    std::jthread reminderPollerThread(poll_thread);

    auto threadHandle = (OSThread *)reminderPollerThread.native_handle();
    OSSetThreadName(threadHandle, "TVii Reminder Poller");
    OSSetThreadAffinity(threadHandle, OS_THREAD_ATTRIB_AFFINITY_ANY);

    reminderPollerThread.detach();
  } catch (std::exception &e) {
    DEBUG_FUNCTION_LINE("Couldn't make thread: %s", e.what());
  }
}
} // namespace reminderpoller