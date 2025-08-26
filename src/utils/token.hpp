#pragma once
#include <wups.h>
#include <nn/act/client_cpp.h>

namespace token
{
    extern std::string currentReplacementToken;

    void setReplacementToken(std::string token, nn::act::SlotNo slot);
    void initToken();
    void resetTokens();
    void updCurrentToken();
} // namespace token
