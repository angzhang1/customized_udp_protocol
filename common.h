/**
 * @author Ang Zhang
 * @student_id W1287478
 *
 * Common definitions for the project
 */
#ifndef COMMON_H_
#define COMMON_H_

#include <arpa/inet.h>
#include <stdint.h>

static const uint16_t PACKET_START_IDENTIFIER = 0xffff;
static const uint16_t PACKET_END_IDENTIFIER = 0xffff;

// 264 bytes maximum message size
#define MAX_MSG_LENGTH 264
#define MAX_PAYLOAD_LENGTH 255

// Defines the serve ip address and port
static const char SERVER_IP[] = "127.0.0.1";
static const uint16_t SERVER_PORT = 8009u;

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
} DataPacket_t;

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

// Write packet to a message (memory already allocated enough)
// Return the size
size_t WriteDataPacket(const DataPacket_t* data_pack, uint8_t* buffer);
size_t WriteAckPacket(uint8_t client_id, uint8_t segment, uint8_t* buffer);
size_t WriteRejectPacket(uint8_t client_id, uint8_t segment,
                         uint16_t reject_code, uint8_t* buffer);

// Read certain information from a buffer with specified format
PACKET_TYPE ReadType(const uint8_t* buffer);

// Helper function to read the header information
Header ReadHeader(const uint8_t* buffer);

// Read a data packet from buffer according to the format
// Return the size of the data packet received.
int ReadDataPacket(const uint8_t* buffer, int buf_len, DataPacket_t* data);

AckPacket_t ReadAckPacket(const uint8_t* buffer);

RejectPacket_t ReadRejectPacket(const uint8_t* buffer);

// Message Definitions for Assignment2

typedef enum {
  ACC_PER = 0xfff8,
  NOT_PAID = 0xfff9,
  NOT_EXIST = 0xfffa,
  ACCESS_OK = 0xfffb,
} VERIFY_TYPE;

typedef struct {
  uint16_t start_id_;
  uint8_t client_id_;
  uint16_t status_;
  uint8_t segment_;
  uint8_t length_;
  uint8_t technology_;
  uint32_t subscriber_;
  uint16_t end_id_;
} __attribute__((packed)) VerificationPacket_t;

VerificationPacket_t ReadVerificationPacket(const uint8_t* buffer);

size_t WriteVerificationPacket(const VerificationPacket_t* packet,
                               uint8_t* buffer);

#endif  // COMMON_H_
