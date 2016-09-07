/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: network_config.proto */

#ifndef PROTOBUF_C_network_5fconfig_2eproto__INCLUDED
#define PROTOBUF_C_network_5fconfig_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1002001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _NetworkConfig NetworkConfig;


/* --- enums --- */


/* --- messages --- */

struct  _NetworkConfig
{
  ProtobufCMessage base;
  int32_t port;
  int32_t number_of_conn;
  int32_t keep_time;
  protobuf_c_boolean is_enable_cache;
  int32_t cache_count;
};
#define NETWORK_CONFIG__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&network_config__descriptor) \
    , 0, 0, 0, 0, 0 }


/* NetworkConfig methods */
void   network_config__init
                     (NetworkConfig         *message);
size_t network_config__get_packed_size
                     (const NetworkConfig   *message);
size_t network_config__pack
                     (const NetworkConfig   *message,
                      uint8_t             *out);
size_t network_config__pack_to_buffer
                     (const NetworkConfig   *message,
                      ProtobufCBuffer     *buffer);
NetworkConfig *
       network_config__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   network_config__free_unpacked
                     (NetworkConfig *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*NetworkConfig_Closure)
                 (const NetworkConfig *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor network_config__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_network_5fconfig_2eproto__INCLUDED */