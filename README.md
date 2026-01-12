# Telemetry4SoC

Telemetry4SoC is a small C/C++ telemetry framework for System-on-Chip style
projects. It provides a lock-free-ish ring buffer for events, a background
agent that drains events, and transport adapters to send them out.

## Features
- Fixed-size telemetry events with payload, level, and monotonic timestamp.
- Thread-safe ring buffer for producer/consumer usage.
- Telemetry agent that drains the ring buffer in a background thread.
- C/C++ transport interface with a C adapter for C++ transports.
- OS abstraction layer (Linux implementations included).
- Example app and basic unit tests.

## Project Layout
- `core/` event definition, ring buffer, memory pool (stub).
- `agent/` telemetry agent that drains the ring buffer.
- `transport/` transport interfaces, adapter, mock transport, UDP transport.
- `os/` OS abstraction layer headers and Linux implementations.
- `api/` public API headers (some stubs).
- `example/` demo application using the UDP transport.
- `tests/` unit tests for events and ring buffer.

## Build
Requirements: CMake 3.10+, C11/C++17, pthreads on Linux.

```bash
mkdir -p build
cmake -S . -B build
cmake --build build
```

### Build Options
- `TELEMETRY_BUILD_EXAMPLES` (default: ON)
- `TELEMETRY_BUILD_TESTS` (default: ON)

Example:
```bash
cmake -S . -B build -DTELEMETRY_BUILD_TESTS=OFF
cmake --build build
```

## Run
Example app:
```bash
./build/example/telemetry_example
```

Tests:
```bash
./build/tests/test_telemetry_framework
```

## Usage Sketch
```cpp
ring_buffer_t* rb = nullptr;
ring_buffer_init(&rb, 1024);

transport::UdpTransport udp;
transport::Config cfg{};
cfg.endpoint = "127.0.0.1:9000";
cfg.mtu = 900;
udp.Init(cfg);

transport_c_t c_transport = transport_adapter::make_transport_adapter(udp);

telemetry_agent_t* agent = nullptr;
telemetry_agent_start(&agent, rb, &c_transport);

telemetry_event_t ev{};
ev.event_id = 1;
ev.level = TELEMETRY_LEVEL_INFO;
ev.payload_size = 0;
ev.timestamp = 123;

ring_buffer_push(rb, &ev);
telemetry_agent_notify(agent);

telemetry_agent_stop(agent);
udp.shutdown();
ring_buffer_free(rb);
```

## Notes
- `api/telemetry.hpp` and `api/config.hpp` are placeholders at the moment.
- Memory pool, UART transport, and shared memory transport are not considered
  for now and will be implemented later.
