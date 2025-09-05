#pragma once
#include <nn/act/client_cpp.h>
#include <wups.h>

namespace token {
extern std::string currentReplacementToken;

void setReplacementToken(std::string token, nn::act::SlotNo slot);
void initToken();
void updCurrentToken();
void resetTokens();
} // namespace token
