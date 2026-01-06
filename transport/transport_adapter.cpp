#include "stdbool.h"
#include "transport_adapter.hpp"


namespace transport_adapter {

    static bool send_event_adapter(void* context, const telemetry_event_t* event) 
    {
        if (context == NULL || event == NULL) 
            return false;

        auto* transport = static_cast<transport::ITransport*>(context);

        return transport->sendEvent(*event);
    }

    static void shutdown_event_adapter(void* context) 
    {
        if(context == NULL)
            return;

        auto* transport = static_cast<transport::ITransport*>(context);
        transport->shutdown();
        
    }

    transport_c_t make_transport_adapter(transport::ITransport& transport_obj) 
    {
        transport_c_t transport{};

        transport.context = &transport_obj;
        transport.send_event = send_event_adapter;
        transport.shutdown = shutdown_event_adapter;
        
        return transport;
    }

}