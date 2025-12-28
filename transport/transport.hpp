#pragma once

#include <cstddef>
#include <cstdint>


extern "C" {
    #include "core/event.h"
}

namespace transport {

struct Config
{
    // declare the memeber for port access and chunksize
    const char* endpoint = NULL;        // later have to defined the port address here
    uint32_t mtu = 512;                 // Maximum transmission unit/ chunksize
};

// Interface for Transport
class ITransport{
public:
    virtual ~ITransport() = default;
    virtual bool Init(const Config &cfg) = 0;
    virtual bool sendEvent(const telemetry_event_t &event) = 0;
    virtual bool shutdown() = 0;
};
}
