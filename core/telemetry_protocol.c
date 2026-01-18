/**
 * @file telemetry_protocol.c
 * @brief Telemetry protocol encoding/decoding implementation.
 *
 * Implements binary serialization and deserialization of telemetry protocol messages
 * using big-endian byte ordering. Provides helper functions for integer conversions
 * and message header encoding/decoding.
 *
 * @author Aravinthraj Ganesan
 */

#include "telemetry_protocol.h"


/**
 * @brief Convert unsigned 16-bit integer to big-endian format.
 *
 * @param[in] value Value in native byte order
 * @return Value converted to big-endian byte order
 */
static inline uint16_t to_be_u16(uint16_t value)
{
    value = ((value >> 8) | (value << 8));
    return value;
}

/**
 * @brief Convert unsigned 32-bit integer to big-endian format.
 *
 * @param[in] value Value in native byte order
 * @return Value converted to big-endian byte order
 */
static inline uint32_t to_be_u32(uint32_t value)
{
    value = (((value & 0x000000FFu) << 24) |
             ((value & 0x0000FF00u) << 8)  |
             ((value & 0x00FF0000u) >> 8)  |
             ((value & 0xFF000000u) >> 24) 
            );

    return value;
}

/**
 * @brief Convert unsigned 64-bit integer to big-endian format.
 *
 * @param[in] value Value in native byte order
 * @return Value converted to big-endian byte order
 */
static inline uint64_t to_be_u64(uint64_t value)
{
    value = (((value & 0x00000000000000FFULL) << 56) |
             ((value & 0x000000000000FF00ULL) << 40) |
             ((value & 0x0000000000FF0000ULL) << 24) |
             ((value & 0x00000000FF000000ULL) << 8)  |
             ((value & 0x000000FF00000000ULL) >> 8)  |
             ((value & 0x0000FF0000000000ULL) >> 24) |
             ((value & 0x00FF000000000000ULL) >> 40) |
             ((value & 0xFF00000000000000ULL) >> 56) 
            );
    return value;
}

/**
 * @brief Write unsigned 8-bit integer to buffer.
 *
 * @param[out] buffer Destination buffer
 * @param[in]  value  Value to write
 */
static inline void put_u8(uint8_t* buffer, uint8_t value)
{
    buffer[0] = value;
}

/**
 * @brief Write unsigned 16-bit integer to buffer in big-endian format.
 *
 * @param[out] buffer Destination buffer (must hold at least 2 bytes)
 * @param[in]  value  Value to write in native byte order
 */
static inline void put_u16_be(uint8_t* buffer, uint16_t value)
{
    /* Convert to big-endian format */
    uint16_t buff_bigend = to_be_u16(value);

    /* Write bytes to buffer */
    buffer[0] = ((buff_bigend >> 0x08) & 0xFF);
    buffer[1] = ((buff_bigend & 0xFF));
}

/**
 * @brief Write unsigned 32-bit integer to buffer in big-endian format.
 *
 * @param[out] buffer Destination buffer (must hold at least 4 bytes)
 * @param[in]  value  Value to write in native byte order
 */
static inline void put_32_be(uint8_t* buffer, uint32_t value)
{
    /* Convert to big-endian format */
    uint32_t buff_bigend = to_be_u32(value);

    /* Write bytes to buffer */
    buffer[0] = ((buff_bigend >> 24) & 0xFF);
    buffer[1] = ((buff_bigend >> 16) & 0xFF);
    buffer[2] = ((buff_bigend >> 8) & 0xFF);
    buffer[3] = (buff_bigend & 0xFF);
}

/**
 * @brief Write unsigned 64-bit integer to buffer in big-endian format.
 *
 * @param[out] buffer Destination buffer (must hold at least 8 bytes)
 * @param[in]  value  Value to write in native byte order
 */
static inline void put_64_be(uint8_t* buffer, uint64_t value)
{
    /* Convert to big-endian format */
    uint64_t buff_bigend = to_be_u64(value);

    /* Write bytes to buffer */
    buffer[0] = ((buff_bigend >> 56) & 0xFF);
    buffer[1] = ((buff_bigend >> 48) & 0xFF);
    buffer[2] = ((buff_bigend >> 40) & 0xFF);
    buffer[3] = ((buff_bigend >> 32) & 0xFF);
    buffer[4] = ((buff_bigend >> 24) & 0xFF);
    buffer[5] = ((buff_bigend >> 16) & 0xFF);
    buffer[6] = ((buff_bigend >> 8) & 0xFF);
    buffer[7] = (buff_bigend & 0xFF);
}

/**
 * @brief Read unsigned 8-bit integer from buffer.
 *
 * @param[in] buffer Source buffer
 * @return Value read from buffer
 */
static inline uint8_t get_u8(const uint8_t* buffer)
{
    uint8_t value = (uint8_t)buffer[0];
    return value;
}

/**
 * @brief Read unsigned 16-bit integer from buffer in big-endian format.
 *
 * @param[in] buffer Source buffer (must hold at least 2 bytes)
 * @return Value converted from big-endian to native byte order
 */
static inline uint16_t get_u16_be(const uint8_t* buffer)
{
    uint16_t value = 0;

    value  = (uint16_t)(buffer[0] << 0x08);
    value |= (uint16_t)buffer[1];

    return value;
}

/**
 * @brief Read unsigned 32-bit integer from buffer in big-endian format.
 *
 * @param[in] buffer Source buffer (must hold at least 4 bytes)
 * @return Value converted from big-endian to native byte order
 */
static inline uint32_t get_u32_be(const uint8_t* buffer)
{
    uint32_t value = 0;

    value  = (uint32_t)(buffer[0] << 24);
    value |= (uint32_t)(buffer[1] << 16);
    value |= (uint32_t)(buffer[2] << 8);
    value |= (uint32_t)(buffer[3]);

    return value;
}

/**
 * @brief Read unsigned 64-bit integer from buffer in big-endian format.
 *
 * @param[in] buffer Source buffer (must hold at least 8 bytes)
 * @return Value converted from big-endian to native byte order
 */
static inline uint64_t get_u64_be(const uint8_t* buffer)
{
    uint64_t value = 0;

    value  = ((uint64_t)buffer[0] << 56);
    value |= ((uint64_t)buffer[1] << 48); 
    value |= ((uint64_t)buffer[2] << 40); 
    value |= ((uint64_t)buffer[3] << 32); 
    value |= ((uint64_t)buffer[4] << 24); 
    value |= ((uint64_t)buffer[5] << 16); 
    value |= ((uint64_t)buffer[6] << 8); 
    value |= ((uint64_t)buffer[7]); 
    return value;
}

// Offsets enum for telemetry header fields
typedef enum telemetry_header_v1_offsets_e {
    OFFSET_MAGIC_VALUE          = 0,
    OFFSET_PROTOCOL_VERSION     = 4,
    OFFSET_HEADER_LENGTH        = 5,
    OFFSET_MESSAGE_TYPE         = 6,
    OFFSET_SEQUENCE_NUMBER      = 8,
    OFFSET_TIMESTAMP            = 12,
    OFFSET_PAYLOAD_LENGTH       = 20,
    OFFSET_CRC32                = 24,
    OFFSET_RESERVED             = 28,
    HEADER_V1_SIZE              = 32
} telemetry_header_v1_offsets_t;


/**     
 * @brief Encode a telemetry header to binary format (v1).
 *
 * Serializes the header structure into big-endian binary format suitable for transmission
 * over the network. Validates all header fields before encoding.
 *
 * @param[out] encoded_buffer   Output buffer to store encoded header
 * @param[in]  buffer_capacity  Size of output buffer in bytes
 * @param[in]  header           Header structure to encode
 * @return Number of bytes written on success (HEADER_V1_SIZE), 0 on error
 *
 * @note Requires buffer_capacity >= HEADER_V1_SIZE (32 bytes)
 */

size_t telemetry_encode_header_v1(uint8_t* encoded_buffer, size_t buffer_capacity, const telemetry_header_t* header)
{
    // Validate input parameters
    if (encoded_buffer == NULL || header == NULL)
        return 0;
    
    // Verify output buffer has sufficient capacity 
    if (buffer_capacity < HEADER_V1_SIZE)
        return 0;
    
    // Validate magic value for protocol identification 
    if (header->magic_value != TELEMETRY_PROTOCOL_MAGIC_VALUE)
        return 0;
    
    // Validate protocol version
    if (header->protocol_version != TELEMETRY_PROTOCOL_VERSION_V1)
        return 0;
    
    // Validate header length field
    if (header->header_length != TELEMETRY_HEADER_LEN)
        return 0;
    
    // Encode the header fields into big-endian format
    put_32_be(&encoded_buffer[OFFSET_MAGIC_VALUE], header->magic_value);
    put_u8(&encoded_buffer[OFFSET_PROTOCOL_VERSION], header->protocol_version);
    put_u8(&encoded_buffer[OFFSET_HEADER_LENGTH], header->header_length);
    put_u16_be(&encoded_buffer[OFFSET_MESSAGE_TYPE], header->message_type);
    put_32_be(&encoded_buffer[OFFSET_SEQUENCE_NUMBER], header->sequence_counter);
    put_64_be(&encoded_buffer[OFFSET_TIMESTAMP], header->timestamp_monotonic_ns);
    put_32_be(&encoded_buffer[OFFSET_PAYLOAD_LENGTH], header->payload_len);
    put_32_be(&encoded_buffer[OFFSET_CRC32]); // CRC32 will be computed later, set to 0 for now
    put_32_be(&encoded_buffer[OFFSET_RESERVED], header->reserved);
        
    return HEADER_V1_SIZE;
}

int telemetry_decode_header_v1(telemetry_header_t* decoded_header, const uint8_t* buffer, size_t buffer_length)
{
    // Validate input parameters
    if (decoded_header == NULL || buffer == NULL)
        return TELEM_RC_ERR_PARM;
    
    // Verify input buffer has sufficient length
    if (buffer_length < HEADER_V1_SIZE)
        return TELEM_RC_ERR_TRUNC;
    
    // Decode the header fields from big-endian format
    decoded_header->magic_value           = get_u32_be(&buffer[OFFSET_MAGIC_VALUE]);
    decoded_header->protocol_version      = get_u8(&buffer[OFFSET_PROTOCOL_VERSION]);
    decoded_header->header_length         = get_u8(&buffer[OFFSET_HEADER_LENGTH]);
    decoded_header->message_type          = get_u16_be(&buffer[OFFSET_MESSAGE_TYPE]);
    decoded_header->sequence_counter      = get_u32_be(&buffer[OFFSET_SEQUENCE_NUMBER]);
    decoded_header->timestamp_monotonic_ns= get_u64_be(&buffer[OFFSET_TIMESTAMP]);
    decoded_header->payload_len           = get_u32_be(&buffer[OFFSET_PAYLOAD_LENGTH]);
    decoded_header->crc32                 = get_u32_be(&buffer[OFFSET_CRC32]);
    decoded_header->reserved              = get_u32_be(&buffer[OFFSET_RESERVED]);
    
    // Validate magic value for protocol identification 
    if (decoded_header->magic_value != TELEMETRY_PROTOCOL_MAGIC_VALUE)
        return TELEM_RC_ERR_MAGIC;
    
    // Validate protocol version
    if (decoded_header->protocol_version != TELEMETRY_PROTOCOL_VERSION_V1)
        return TELEM_RC_ERR_VERSION;
    
    // Validate header length field
    if (decoded_header->header_length != TELEMETRY_HEADER_LEN)
        return TELEM_RC_ERR_HEADER_LEN;

    // Range checks: payload length should not exceed maximum allowed
    if(decoded_header->payload_len > (buffer_length)- sizeof(decoded_header->header_length))
        return TELEM_RC_ERR_RANGE;
    
    if(decoded_header.header_length != HEADER_V1_SIZE)
        return TELEM_RC_ERR_HEADER_LEN;
        
    return TELEM_RC_OK;
}