#include "ring_buffer.h"
#include "transport.hpp"
#include "transport_adapter.hpp"
#include "mock_transport.hpp"
#include "telemetry_agent.h"
#include "udp_transport.hpp"


int main()
{

    // Create and Initalize the ring buffer
    ring_buffer_t* rb = nullptr;
    ring_buffer_init(&rb, 1024);

    // Create the Mock transport C++
    /*transport::MockTransport mock(true);
    transport::Config Cfg{};
    mock.Init(Cfg);*/

    // Create the transport in C++
    transport::UdpTransport udp;
    transport::Config Cfg{};
    Cfg.endpoint = "127.0.0.1:9000";
    Cfg.mtu = 900;

    // Initialize the udp
    udp.Init(Cfg);

    // Wrapper for C++ transport to C
    transport_c_t c_transport = transport_adapter::make_transport_adapter(udp);

    // Start the agent
    telemetry_agent_t* agent = nullptr;
    
    if(telemetry_agent_start(&agent, rb, &c_transport) == false)
    {
        // if the agent is not started successfull then return
        return 1;
    }

    // produce one event 
    telemetry_event_t ev{};
    ev.event_id = 1;
    ev.level = 2;
    ev.payload_size = 0;
    ev.timestamp = 123;

    if(ring_buffer_push(rb, &ev) == true)
    {
        telemetry_agent_notify(agent);
    }

    // stop the agent
    telemetry_agent_stop(agent);

    udp.shutdown();
    ring_buffer_free(rb);

    return 0;
}

