#pragma once
#include <functional>
#include "../common/json/json.hpp"

using json = nlohmann::json;

typedef std::function<void(json const& Message)> SendToBackendDelegate;
typedef std::function<bool(json const& Message)> MessageFromBackendDelegate;

class IBackendConnect
{
private:
    /* data */
public:
    virtual ~IBackendConnect();

    virtual void RegisterMeForBackendMessage(MessageFromBackendDelegate callback) = 0;
};


