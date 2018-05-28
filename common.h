/**
Common definitions for the project
**/
#ifndef COMMON_H_
#define COMMON_H_

#include <arpa/inet.h>
#include <stdint.h>

static const uint16_t PACKET_START_IDENTIFIER = 0xffff;
static const uint16_t PACKET_END_IDENTIFIER = 0xffff;

// 264 bytes maximum message size
static const size_t MAX_MSG_LENGTH = 264;
static const size_t MAX_PAYLOAD_LENGTH = 255;

typedef enum {
  DATA = 0xfff1,
  ACK = 0xfff2,
  REJECT = 0xfff3,
  UNKNOWN = 0,
} PACKET_TYPE;

typedef enum {
  OUT_OF_SEQUENCE = 0xfff4,
  LENGTH_MISMATCH = 0Xfff5,
  END_OF_PACKET_MISSING = 0xfff6,
  DUPLICATE_PACKET = 0xfff7,
} REJECT_REASON;

typedef struct {
  uint16_t start_id_;
  uint8_t client_id_;
  uint16_t type_;
} __attribute__((packed)) Header;

typedef struct {
  Header header_;
  uint8_t segment_;
  uint8_t length_;
  uint8_t payload_[MAX_PAYLOAD_LENGTH];
  uint16_t end_id_;
} __attribute__((packed)) DataPacket_t;

typedef struct {
  Header header_;
  uint8_t recv_segment_;
  uint16_t end_id_;
} __attribute__((packed)) AckPacket_t;

typedef struct {
  Header header_;
  uint16_t reject_code_;
  uint8_t recv_segment_;
  uint16_t end_id_;
} __attribute__((packed)) RejectPacket_t;

static const char SERVER_IP[] = "127.0.0.1";
static const uint16_t SERVER_PORT = 8009u;

// Write packet to a message (memory already allocated enough)
// Return the size
size_t WriteDataPacket(const DataPacket_t* data_pack, uint8_t* buffer);
size_t WriteAckPacket(uint8_t client_id, uint8_t segment, uint8_t* buffer);
size_t WriteRejectPacket(uint8_t client_id, uint8_t segment,
                         uint16_t reject_code, uint8_t* buffer);

PACKET_TYPE ReadType(const uint8_t* buffer);
Header ReadHeader(const uint8_t* buffer);
int ReadDataPacket(const uint8_t* buffer, int buf_len, DataPacket_t* data);
AckPacket_t ReadAckPacket(const uint8_t* buffer);
RejectPacket_t ReadRejectPacket(const uint8_t* buffer);

#endif  // COMMON_H_
