/**
 * @file telemetry_protocol.h
 * @brief Telemetry wire protocol definitions.
 *
 * Defines the telemetry protocol structures and functions for encoding/decoding
 * protocol headers and messages over UDP.
 *
 * Protocol Details:
 * - Version: 1.0
 * - Wire Endianness: Big-endian
 * - Header Size: Fixed 32 bytes for v1
 *
 * @author Aravinthraj Ganesan
 */

#pragma once

#ifdef __cplusplus
    extern "C" {
#endif


#define TELEMETRY_PROTOCOL_VERSION_V1                (uint8_t)1u
#define TELEMETRY_HEADER_LEN                         (uint8_t)32u
#define TELEMETRY_PROTOCOL_MAGIC_VALUE               (uint32_t)0x54454C31u  // ASCII coded : TEL1

/**
 * @struct telemetry_header_s
 * @brief Telemetry message header structure (32 bytes fixed size).
 */
typedef struct telemetry_header_s {
    /** Magic value for fast validation of protocol messages */
    uint32_t magic_value;
    /** Protocol version number */
    uint8_t  protocol_version;
    /** Header length in bytes */
    uint8_t  header_length;
    /** Message type identifier */
    uint16_t message_type;
    /** Sequence counter for message tracking */
    uint32_t sequence_counter;
    /** Timestamp in nanoseconds (monotonic) */
    uint64_t timestamp_monotonic_ns;
    /** Payload length in bytes */
    uint32_t payload_len;
    /** CRC32 checksum for integrity verification */
    uint32_t crc32;
    /** Reserved for future use */
    uint32_t reserved;
} telemetry_header_t;

/**
 * @enum telemetry_msg_type_e
 * @brief Telemetry message types supported by the protocol.
 */
typedef enum telemetry_msg_type_e {
    /** Event batch message type */
    TELEMETRY_EVENT_BATCH       = 1,
    /** Heartbeat/keepalive batch message type */
    TELEMETRY_HEART_BEAT_BATCH  = 2,
    /** Metrics batch message type */
    TELEMETRY_METRICS_BATCH     = 3
} telemetry_msg_type_t;

/**
 * @enum telemetry_rc_type
 * @brief Return codes for telemetry protocol functions.
 */
typedef enum telemetry_rc_type {
    /** Operation completed successfully */
    TELEM_RC_OK = 0,
    /** Invalid or bad argument provided */
    TELEM_RC_ERR_PARM = -1,
    /** Output buffer capacity is insufficient */
    TELEM_RC_ERR_CAPACITY = -2,
    /** Input buffer is shorter than expected */
    TELEM_RC_ERR_TRUNC = -3,
    /** Invalid magic value - not a valid protocol message */
    TELEM_RC_ERR_MAGIC = -4,
    /** Unsupported or invalid protocol version */
    TELEM_RC_ERR_VERSION = -5,
    /** Invalid header length field */
    TELEM_RC_ERR_HEADER_LEN = -6,
    /** Value is out of valid range */
    TELEM_RC_ERR_RANGE = -7
} telemetry_rc_type_t;


/**
 * @brief Encode a telemetry header to binary format (v1).
 *
 * Serializes the header structure into big-endian binary format suitable for transmission.
 *
 * @param[out] encoded_buffer   Output buffer to store encoded header
 * @param[in]  buffer_capacity  Size of output buffer in bytes
 * @param[in]  header           Header structure to encode
 * @return Number of bytes written on success, 0 on error
 */
size_t telemetry_encode_header_v1(uint8_t* encoded_buffer, size_t buffer_capacity, const telemetry_header_t* header);

/**
 * @brief Decode a telemetry header from binary format (v1).
 *
 * Deserializes a big-endian binary header into the header structure.
 * Validates magic value and version before decoding.
 *
 * @param[out] decoded_header  Pointer to decoded header structure
 * @param[in]  buffer          Binary header data buffer received
 * @param[in]  buffer_length   Length of received buffer in bytes
 * @return TELEM_RC_OK on success, error code on failure
 */
int telemetry_decode_header_v1(telemetry_header_t* decoded_header, const uint8_t* buffer, size_t buffer_length);



/**
 * @brief Get the fixed header length for telemetry protocol v1.
 *
 * Returns the fixed size of a telemetry header in bytes. This is a compile-time
 * constant but wrapped in a function for API consistency and type safety.
 *
 * @return Size of telemetry header v1 in bytes (always 32 bytes)
 */
/**
 * @brief Get the fixed header length for telemetry protocol v1.
 *
 * Returns the fixed size of a telemetry header in bytes. This is a compile-time
 * constant but wrapped in a function for API consistency and type safety.
 *
 * @return Size of telemetry header v1 in bytes (always 32 bytes)
 */
static inline size_t telemetry_header_v1_length(void) {
    return (size_t)(TELEMETRY_HEADER_LEN);
}

#ifdef __cplusplus
    }
#endif