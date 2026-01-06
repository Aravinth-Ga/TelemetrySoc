#pragma once

#include <stdbool.h>
#include "../core/event.h"

#ifdef __cplusplus
    extern "C" {
#endif


// C friendly trasnport interface
typedef struct transport_c {

    // points to a C++ ITransport instance
    void* context;                                                      

    // Function pointer to send an event
    bool (*send_event)(void* context, const telemetry_event_t* ev);

    // function pointer for shutdown
    void (*shutdown)(void* context);  

}transport_c_t;


#ifdef __cplusplus
    }
#endif