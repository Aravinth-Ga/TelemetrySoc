# Telemetry4SoC

Telemetry4SoC is a small C/C++ telemetry framework for System-on-Chip style
projects. It provides a lock-free-ish ring buffer for events, a background
agent that drains events, and transport adapters to send them out.

## Features
- Fixed-size telemetry events with payload, level, and monotonic timestamp.
- Thread-safe ring buffer for producer/consumer usage with drop tracking.
- Telemetry agent that drains the ring buffer in a background thread.
- C++ transport interface with a C adapter; mock and UDP transports included.
- UDP transport sends JSON events to a configured endpoint.
- OS abstraction layer (Linux implementations included).
- Example app that sends events over UDP; unit tests for core components.

## Project Layout
- `core/` event definition, ring buffer, memory pool (placeholder).
- `agent/` telemetry agent that drains the ring buffer.
- `transport/` transport interfaces, adapter, mock transport, UDP transport.
- `os/` OS abstraction layer headers and Linux implementations.
- `api/` public API headers (C++ headers are placeholders).
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

## Quick Terminal Smoke Test (UDP)
1) Build:
```bash
cmake -S . -B build
cmake --build build
```

2) Start a UDP listener (pick one):
```bash
nc -u -l 9000
```
```bash
socat -u UDP-RECV:9000 STDOUT
```

3) Run the example in another terminal:
```bash
./build/example/telemetry_example
```

You should see JSON-formatted events printed by the UDP listener.

## Run Only Prebuilt Executables
If you already compiled the binaries, you can just run them directly:
```bash
./build/example/telemetry_example
```

For the UDP console receiver, start your receiver binary first, then run the example:
```bash
./path/to/udp_console_receiver
./build/example/telemetry_example
```

## Usage Sketch
```cpp
ring_buffer_t* rb = nullptr;
ring_buffer_init(&rb, 1024);

transport::UdpTransport udp;
transport::Config cfg{};
cfg.endpoint = "127.0.0.1:9000";
cfg.mtu = 512;
udp.Init(cfg);

transport_c_t c_transport = transport_adapter::make_transport_adapter(udp);

telemetry_agent_t* agent = nullptr;
telemetry_agent_start(&agent, rb, &c_transport);

telemetry_event_t ev{};
ev.event_id = 1;
ev.level = TELEMETRY_LEVEL_INFO;
ev.payload_size = 0;
ev.timestamp = osal_telemetry_now_monotonic_ns();

ring_buffer_push(rb, &ev);
telemetry_agent_notify(agent);

telemetry_agent_stop(agent);
udp.shutdown();
ring_buffer_free(rb);
```

## Current Status
- UDP transport is implemented with JSON serialization and basic socket setup.
- Example uses UDP; mock transport remains available for tests.
- `api/telemetry.hpp`, `api/config.hpp`, and `api/telemetry.cpp` are placeholders.
- Memory pool files in `core/` are empty placeholders.
- Tests currently cover events and the ring buffer only.
