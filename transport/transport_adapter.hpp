#pragma once

#include "transport_c.h"
#include "transport.hpp"

namespace transport_adapter {


transport_c_t make_transport_adapter(transport::ITransport& transport_obj);
    
}