# Telemetry4SoC User Manual

This manual explains the Telemetry4SoC project for readers new to telemetry
systems and this codebase. It covers system concepts, data flow, and every
public API with arguments, return values, and usage notes.

## 1. Purpose and overview

Telemetry4SoC is a small C and C++ telemetry framework designed for
System-on-Chip style projects. It collects telemetry events in a lock-free
ring buffer, then uses a background agent thread to drain events and send them
through a transport such as UDP. The transport layer is pluggable, with a C++
interface and a C adapter so the C agent can call any transport.

## 2. Project layout

- `core/` event definitions and the ring buffer implementation.
- `agent/` background telemetry agent that drains the ring buffer.
- `transport/` transport interfaces, C adapter, mock transport, UDP transport.
- `os/` OS abstraction layer for thread, wakeup, and time.
- `api/` public type definitions and placeholder C++ API headers.
- `example/` demo application using the mock transport.
- `tests/` unit tests for events and ring buffer behavior.
- `docs/` project documentation including this manual.

## 3. Build and run

Requirements:
- CMake 3.10 or newer
- C11 and C++17 compilers
- pthreads (Linux)

Build steps:
```bash
mkdir -p build
cmake -S . -B build
cmake --build build
```

Build options:
- `TELEMETRY_BUILD_EXAMPLES` controls example build, default ON.
- `TELEMETRY_BUILD_TESTS` controls tests build, default ON.

Run the example:
```bash
./build/example/telemetry_example
```

Run the tests:
```bash
./build/tests/test_telemetry_framework
```

## 4. Core concepts and data flow

### 4.1 Telemetry events

An event is a fixed-size record consisting of an event id, severity level,
payload size, timestamp, and payload bytes. The payload is capped at 128 bytes.
The event timestamp is taken from a monotonic clock, which is not affected by
system clock changes.

### 4.2 Ring buffer

The ring buffer is a fixed-size queue used for producer-consumer telemetry
events. It uses atomics so producers and the consumer can safely operate
concurrently. One extra slot is allocated to distinguish full versus empty.
When the ring is full, pushes fail and a dropped counter increments.

### 4.3 Telemetry agent

The telemetry agent runs in a background thread. Producers push events to the
ring buffer and notify the agent. The agent wakes, drains events from the ring
buffer, and sends them via the transport. The agent is designed to keep
producer threads fast and non-blocking.

### 4.4 Transport

Transport is the output layer that sends events outside the device. It is a
C++ interface with a C adapter so the C agent can call any C++ transport
implementation. The mock transport is used for tests and the example. The UDP
transport has socket setup and endpoint parsing implemented, but serialization
and send are still in progress.

## 5. Public API reference

Unless explicitly noted, functions return false on invalid input, and return
true on success.

### 5.1 `api/type.h`

Purpose: fixed width integer types used by the project.

- `typedef __UINT8_TYPE__  uint8_t`  
  Description: Unsigned 8 bit integer type.
- `typedef __UINT16_TYPE__ uint16_t`  
  Description: Unsigned 16 bit integer type.
- `typedef __UINT32_TYPE__ uint32_t`  
  Description: Unsigned 32 bit integer type.
- `typedef __UINT64_TYPE__ uint64_t`  
  Description: Unsigned 64 bit integer type.

### 5.2 `core/event.h`

Purpose: defines telemetry events and helper functions.

Macro:
- `TELEMETRY_EVENT_PAYLOAD_MAX`  
  Maximum payload bytes per event. Default is 128 if not defined before
  including the header.

Enum:
- `telemetry_level_t`  
  Values: `TELEMETRY_LEVEL_DEBUG`, `TELEMETRY_LEVEL_INFO`,
  `TELEMETRY_LEVEL_WARNING`, `TELEMETRY_LEVEL_ERROR`.  
  Description: Severity level for telemetry events.

Struct:
- `telemetry_event_t`  
  Fields:
  - `event_id` `uint32_t` application defined event identifier.
  - `level` `uint8_t` severity level from `telemetry_level_t`.
  - `reserved` `uint8_t` padding for alignment.
  - `payload_size` `uint16_t` number of valid bytes in payload.
  - `timestamp` `uint64_t` monotonic time in nanoseconds.
  - `payload` `uint8_t[TELEMETRY_EVENT_PAYLOAD_MAX]` raw payload bytes.  
  Description: Fixed size telemetry event structure.

Function:
```c
bool telemetry_event_make(telemetry_event_t* event,
                          uint32_t event_id,
                          const void* payload,
                          size_t payload_size,
                          telemetry_level_t level)
```
Parameters:
- `event` output event to fill. Must not be NULL.
- `event_id` application specific event identifier.
- `payload` payload bytes. May be NULL only if `payload_size` is 0.
- `payload_size` number of payload bytes. Must be 0 to
  `TELEMETRY_EVENT_PAYLOAD_MAX` inclusive.
- `level` severity level.
Returns:
- `true` on success.
- `false` on invalid parameters, including NULL event, payload_size too large,
  or payload is NULL while payload_size is nonzero.
Behavior:
- Sets event fields, copies payload if present, and sets a monotonic timestamp
  using `osal_telemetry_now_monotonic_ns()`.

Function:
```c
size_t telemetry_event_payload_max(void)
```
Parameters: none.
Returns: the maximum payload size in bytes.
Behavior: inline helper that returns `TELEMETRY_EVENT_PAYLOAD_MAX`.

### 5.3 `core/ring_buffer.h`

Purpose: lock-free style ring buffer for telemetry events.

Type:
- `ring_buffer_t`  
  Description: Opaque ring buffer type. Actual definition is in `ring_buffer.c`.

Function:
```c
bool ring_buffer_init(ring_buffer_t** out_rb, size_t capacity)
```
Parameters:
- `out_rb` output pointer that receives the ring buffer handle.
- `capacity` number of events that can be stored.
Returns:
- `true` on success.
- `false` on invalid input or allocation failure.
Behavior:
- Allocates a ring buffer with capacity plus one extra slot to distinguish full
  from empty. Initializes atomic head, tail, and dropped counters.

Function:
```c
void ring_buffer_free(ring_buffer_t* rb)
```
Parameters:
- `rb` ring buffer handle returned by `ring_buffer_init`.
Returns: no return value.
Behavior:
- Frees internal memory and resets internal fields. Safe to call with NULL.

Function:
```c
bool ring_buffer_push(ring_buffer_t* rb, telemetry_event_t* event)
```
Parameters:
- `rb` ring buffer handle.
- `event` input event to copy into the ring buffer.
Returns:
- `true` when the event is pushed.
- `false` when input is invalid or the ring is full.
Behavior:
- Reads head and tail atomics. If full, increments dropped counter and returns
  false. Otherwise copies the event into the buffer and advances head.

Function:
```c
bool ring_buffer_pop(ring_buffer_t* rb, telemetry_event_t* out)
```
Parameters:
- `rb` ring buffer handle.
- `out` output event to receive the popped event.
Returns:
- `true` when an event is available and copied to `out`.
- `false` when input is invalid or the ring is empty.
Behavior:
- Reads head and tail atomics. If empty, returns false. Otherwise copies the
  event from the buffer and advances tail.

Function:
```c
size_t ring_buffer_count(const ring_buffer_t* rb)
```
Parameters:
- `rb` ring buffer handle.
Returns:
- Number of events currently in the ring buffer. Returns 0 on invalid input.
Behavior:
- Computes count based on head and tail indices, accounting for wrap around.

Function:
```c
uint64_t ring_buffer_dropped(const ring_buffer_t* rb)
```
Parameters:
- `rb` ring buffer handle.
Returns:
- Number of events dropped due to full buffer. Returns 0 on invalid input.
Behavior:
- Reads the dropped counter atomically.

### 5.4 `agent/telemetry_agent.h`

Purpose: background agent that drains the ring buffer and sends events.

Type:
- `telemetry_agent_t`  
  Description: Opaque telemetry agent handle.

Function:
```c
bool telemetry_agent_start(telemetry_agent_t** out_agent,
                           ring_buffer_t* ring_handle,
                           transport_c_t* transport)
```
Parameters:
- `out_agent` receives the created agent handle on success.
- `ring_handle` ring buffer to drain.
- `transport` transport interface used to send events.
Returns:
- `true` on success.
- `false` if inputs are invalid or thread or wakeup creation fails.
Behavior:
- Allocates the agent, creates a wakeup object, starts the consumer thread,
  and stores handles to the ring buffer and transport. The thread waits on
  `osal_wakeup_wait` and drains events on wake.

Function:
```c
void telemetry_agent_stop(telemetry_agent_t* agent)
```
Parameters:
- `agent` telemetry agent handle.
Returns: no return value.
Behavior:
- Signals the thread to stop, wakes it, drains remaining events, joins and
  destroys the thread, destroys the wakeup object, calls transport shutdown,
  and frees the agent. Safe to call with NULL.

Function:
```c
void telemetry_agent_notify(telemetry_agent_t* agent)
```
Parameters:
- `agent` telemetry agent handle.
Returns: no return value.
Behavior:
- Increments the wakeup count and signals the wakeup object so the consumer
  thread can drain events. Safe to call with NULL.

Function:
```c
uint64_t telemetry_agent_sent_count(const telemetry_agent_t* agent)
```
Parameters:
- `agent` telemetry agent handle.
Returns:
- Number of events successfully sent by the agent. Returns 0 on NULL.
Behavior:
- Reads the sent counter atomically.

Function:
```c
uint64_t telemetry_agent_wakeup_count(const telemetry_agent_t* agent)
```
Parameters:
- `agent` telemetry agent handle.
Returns:
- Number of wakeups seen by the agent. Returns 0 on NULL.
Behavior:
- Reads the wakeup counter atomically.

### 5.5 `transport/transport.hpp`

Purpose: C++ transport interface and configuration.

Struct:
- `transport::Config`  
  Fields:
  - `endpoint` `const char*` destination endpoint for transport.
  - `mtu` `uint32_t` max datagram size for transport payload.  
  Description: Transport configuration. Endpoint format is transport specific.

Class:
- `transport::ITransport`

Method:
```cpp
virtual bool Init(const Config& cfg) = 0;
```
Parameters:
- `cfg` transport configuration.
Returns:
- `true` on successful initialization.
- `false` on invalid config or initialization failure.
Behavior:
- Prepares transport resources such as sockets or device handles.

Method:
```cpp
virtual bool sendEvent(const telemetry_event_t& event) = 0;
```
Parameters:
- `event` telemetry event to send.
Returns:
- `true` on successful send.
- `false` on failure.
Behavior:
- Serializes and sends the telemetry event through the transport.

Method:
```cpp
virtual void shutdown() = 0;
```
Parameters: none.
Returns: no return value.
Behavior:
- Releases transport resources and stops the transport.

### 5.6 `transport/transport_c.h`

Purpose: C compatible transport interface for the C agent.

Struct:
- `transport_c_t`  
  Fields:
  - `context` `void*` pointer to a C++ `ITransport` instance.
  - `send_event` function pointer:  
    `bool (*send_event)(void* context, const telemetry_event_t* ev)`
  - `shutdown` function pointer:  
    `void (*shutdown)(void* context)`  
  Description: C struct used by the C agent to call a C++ transport via
  function pointers.

Function pointer:
- `send_event`  
  Parameters:
  - `context` C++ transport instance.
  - `ev` event to send.  
  Returns:
  - `true` on successful send. `false` on failure.  
  Behavior:
  - Calls the C++ transport `sendEvent` implementation.

Function pointer:
- `shutdown`  
  Parameters:
  - `context` C++ transport instance.  
  Returns: no return value.  
  Behavior:
  - Calls the C++ transport `shutdown` implementation.

### 5.7 `transport/transport_adapter.hpp`

Purpose: adapter to convert a C++ `ITransport` into `transport_c_t`.

Function:
```cpp
transport_c_t make_transport_adapter(transport::ITransport& transport_obj);
```
Parameters:
- `transport_obj` reference to a C++ transport implementation.
Returns:
- `transport_c_t` with function pointers wired to call `transport_obj`.
Behavior:
- Creates a `transport_c_t` with context set to `transport_obj` and
  `send_event` and `shutdown` pointers set to adapter functions that call the
  C++ methods.

### 5.8 `transport/mock_transport.hpp`

Purpose: mock transport for tests and example.

Class:
- `transport::MockTransport`

Constructor:
```cpp
explicit MockTransport(bool enable_print = true);
```
Parameters:
- `enable_print` controls whether events are printed when sent.
Behavior:
- Stores the print flag for later use in `sendEvent`.

Method:
```cpp
bool Init(const Config& cfg) override;
```
Parameters:
- `cfg` is ignored by the mock transport.
Returns:
- `true` always.
Behavior:
- Resets the internal sent counter to zero.

Method:
```cpp
bool sendEvent(const telemetry_event_t& event) override;
```
Parameters:
- `event` telemetry event to simulate sending.
Returns:
- `true` always.
Behavior:
- Increments the sent counter. If `enable_print` is true, prints event fields
  to stdout.

Method:
```cpp
void shutdown() override;
```
Parameters: none.
Returns: no return value.
Behavior:
- No operation for mock transport.

Method:
```cpp
uint64_t sendCount() const;
```
Parameters: none.
Returns:
- Number of events sent by the mock transport.
Behavior:
- Reads the atomic counter.

### 5.9 `transport/udp_transport.hpp` and `transport/udp_transport.cpp`

Purpose: UDP based transport. Initialization is implemented. Event
serialization and send are still in progress.

Class:
- `transport::UdpTransport`

Constructor:
```cpp
UdpTransport();
```
Behavior:
- Default constructor. Initializes internal members to default values.

Destructor:
```cpp
~UdpTransport() override;
```
Behavior:
- Destructor declaration exists. Implementation should release resources if
  any remain open.

Method:
```cpp
bool Init(const Config& cfg) override;
```
Parameters:
- `cfg.endpoint` required and must be in `host:port` form.
- `cfg.mtu` maximum datagram size. If zero, default 512 is used. If larger
  than 1200, it is clamped to 1200.
Returns:
- `true` on successful initialization.
- `false` on invalid endpoint, socket setup failure, or address parse failure.
Behavior:
- Validates endpoint, clamps mtu, opens a UDP socket, parses and stores the
  destination address, and marks the transport as ready.

Method:
```cpp
bool sendEvent(const telemetry_event_t& event) override;
```
Parameters:
- `event` telemetry event to send.
Returns:
- `true` on success. `false` on failure.
Behavior:
- Currently not implemented. Expected to serialize the event and call
  `sendto` using the stored destination address.

Method:
```cpp
void shutdown() override;
```
Parameters: none.
Returns: no return value.
Behavior:
- Currently not implemented. Expected to close the UDP socket and reset state.

Private method:
```cpp
bool open_udp_socket();
```
Parameters: none.
Returns:
- `true` on success. `false` on failure.
Behavior:
- Creates a UDP socket and sets a large send buffer. If socket already open,
  returns true without creating a new one.

Private method:
```cpp
bool configure_destination(const char* endpoint);
```
Parameters:
- `endpoint` string in `host:port` form.
Returns:
- `true` on success. `false` on invalid format or parse failure.
Behavior:
- Splits host and port, validates the port range, resolves localhost to
  `127.0.0.1`, converts IP address using `inet_pton`, and stores `sockaddr_in`
  into internal aligned storage.

Private method:
```cpp
bool serialize_event_json(char* out_buf, size_t out_cap,
                          telemetry_event_t& event) const;
```
Parameters:
- `out_buf` output buffer for serialized data.
- `out_cap` capacity of `out_buf` in bytes.
- `event` event to serialize.
Returns:
- `true` on success. `false` on invalid inputs.
Behavior:
- Currently incomplete. Validates buffer and caps payload length to 128 bytes.
  Expected to format JSON into `out_buf` and return true on success.

### 5.10 `os/include/osal_thread.h`

Purpose: OS abstraction for threads.

Type:
- `osal_thread_t`  
  Description: Opaque thread handle.

Type:
- `osal_thread_fn_t`  
  Description: Thread entry function type. Signature is `void* (*)(void*)`.

Function:
```c
int osal_thread_create(osal_thread_t** out_thread,
                       osal_thread_fn_t entry_fn,
                       void* entry_arg,
                       const char* thread_name);
```
Parameters:
- `out_thread` receives the thread handle.
- `entry_fn` thread entry function.
- `entry_arg` argument passed to the entry function.
- `thread_name` optional name for the thread.
Returns:
- `0` on success.
- negative error code on failure.
Behavior:
- Creates a new OS thread and returns its handle.

Function:
```c
int osal_thread_join(osal_thread_t* thread);
```
Parameters:
- `thread` thread handle.
Returns:
- `0` on success.
- negative error code on failure.
Behavior:
- Waits for the given thread to finish.

Function:
```c
void osal_thread_destroy(osal_thread_t* thread);
```
Parameters:
- `thread` thread handle.
Returns: no return value.
Behavior:
- Frees thread resources. Safe to call with NULL.

### 5.11 `os/include/osal_wakeup.h`

Purpose: OS abstraction for wakeup and notification.

Type:
- `osal_wakeup_t`  
  Description: Opaque wakeup handle.

Function:
```c
osal_wakeup_t* osal_wakeup_create(void);
```
Parameters: none.
Returns:
- Pointer to wakeup object or NULL on failure.
Behavior:
- Allocates and initializes wakeup mechanism.

Function:
```c
void osal_wakeup_notify(osal_wakeup_t* wakeup);
```
Parameters:
- `wakeup` wakeup handle.
Returns: no return value.
Behavior:
- Signals a waiting thread to wake. Safe to call with NULL.

Function:
```c
void osal_wakeup_wait(osal_wakeup_t* wakeup);
```
Parameters:
- `wakeup` wakeup handle.
Returns: no return value.
Behavior:
- Blocks the calling thread until notified. Safe to call with NULL.

Function:
```c
void osal_wakeup_destroy(osal_wakeup_t* wakeup);
```
Parameters:
- `wakeup` wakeup handle.
Returns: no return value.
Behavior:
- Releases wakeup resources and frees the object. Safe to call with NULL.

### 5.12 `os/include/osal_time.h`

Purpose: OS abstraction for monotonic time.

Function:
```c
uint64_t osal_telemetry_now_monotonic_ns(void);
```
Parameters: none.
Returns:
- Monotonic time in nanoseconds.
Behavior:
- Uses a monotonic clock source and returns nanoseconds since an unspecified
  start point. This is suitable for measuring elapsed time.

## 6. Example usage walkthrough

This describes the example flow implemented in `example/demo.cpp`.

```cpp
// Step 1: Create a ring buffer.
ring_buffer_t* rb = nullptr;
ring_buffer_init(&rb, 1024);

// Step 2: Create and initialize a transport.
transport::MockTransport mock(true);
transport::Config cfg{};
mock.Init(cfg);

// Step 3: Adapt transport for the C agent.
transport_c_t c_transport = transport_adapter::make_transport_adapter(mock);

// Step 4: Start the telemetry agent.
telemetry_agent_t* agent = nullptr;
telemetry_agent_start(&agent, rb, &c_transport);

// Step 5: Create and push an event.
telemetry_event_t ev{};
telemetry_event_make(&ev, 1, NULL, 0, TELEMETRY_LEVEL_INFO);
ring_buffer_push(rb, &ev);
telemetry_agent_notify(agent);

// Step 6: Stop the agent and clean up.
telemetry_agent_stop(agent);
mock.shutdown();
ring_buffer_free(rb);
```

## 7. Known limitations and planned work

- UDP transport does not yet serialize or send events.
- The C++ API headers in `api/` are placeholders.
- Memory pool, UART transport, and shared memory transport are planned and not
  implemented yet.
- The UDP dashboard receiver is planned and not implemented yet.
