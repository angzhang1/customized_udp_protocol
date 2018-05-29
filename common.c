#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t WriteDataPacket(const DataPacket_t *data_pack, uint8_t *buffer) {
  if (data_pack == NULL) {
    return 0;
  }

  uint8_t *buffer_begin = buffer;
  *(uint16_t *)buffer = PACKET_START_IDENTIFIER;
  buffer += 2;
  *(uint8_t *)buffer = data_pack->header_.client_id_;
  buffer++;
  *(uint16_t *)buffer = DATA;
  buffer += 2;
  *(uint8_t *)buffer = data_pack->segment_;
  buffer++;
  *(uint8_t *)buffer = data_pack->length_;
  buffer++;
  size_t payload_len = strlen((char *)data_pack->payload_) + 1;
  memcpy((void *)buffer, (void *)data_pack->payload_, payload_len);
  buffer += payload_len;
  *(uint16_t *)buffer = data_pack->end_id_;
  buffer += 2;

  return (buffer - buffer_begin);
}

size_t WriteAckPacket(uint8_t client_id, uint8_t segment, uint8_t *buffer) {
  if (buffer == NULL) {
    return 0;
  }
  AckPacket_t ack_pack;
  ack_pack.header_.start_id_ = PACKET_START_IDENTIFIER;
  ack_pack.header_.type_ = ACK;
  ack_pack.header_.client_id_ = client_id;
  ack_pack.recv_segment_ = segment;
  ack_pack.end_id_ = PACKET_END_IDENTIFIER;
  memcpy((void *)buffer, (void *)&ack_pack, sizeof(ack_pack));
  return sizeof(ack_pack);
}

size_t WriteRejectPacket(uint8_t client_id, uint8_t segment,
                         uint16_t reject_code, uint8_t *buffer) {
  if (buffer == NULL) {
    return 0;
  }
  RejectPacket_t rej_pack;
  rej_pack.header_.start_id_ = PACKET_START_IDENTIFIER;
  rej_pack.header_.type_ = REJECT;
  rej_pack.header_.client_id_ = client_id;
  rej_pack.reject_code_ = (REJECT_REASON)reject_code;
  rej_pack.recv_segment_ = segment;
  rej_pack.end_id_ = PACKET_END_IDENTIFIER;
  memcpy((void *)buffer, (void *)&rej_pack, sizeof(rej_pack));
  return sizeof(rej_pack);
}

PACKET_TYPE ReadType(const uint8_t *buffer) {
  PACKET_TYPE type = UNKNOWN;
  if (buffer != NULL) {
    buffer += 3;

    type = *(uint16_t *)buffer;
    // printf("Type: %x, %x \n", type, *(uint16_t *)buffer);
  }
  return type;
}

Header ReadHeader(const uint8_t *buffer) {
  Header hd;
  if (buffer != NULL) {
    memcpy((void *)&hd, buffer, sizeof(hd));
  }
  return hd;
}

int ReadDataPacket(const uint8_t *buffer, int buf_len, DataPacket_t *data) {
  int error = 0;
  if (buffer != NULL && data != NULL) {
    const uint8_t *buffer_begin = buffer;
    data->header_ = ReadHeader(buffer);
    buffer += sizeof(data->header_);
    data->segment_ = *(uint8_t *)buffer;
    buffer++;
    data->length_ = *(uint8_t *)buffer;
    buffer++;

    if (buffer + data->length_ + 2 != buffer_begin + buf_len) {
      error = LENGTH_MISMATCH;
      printf("Length mismatch, will not copy this data!\n");
    } else if (data->length_ < MAX_PAYLOAD_LENGTH) {
      memcpy((void *)data->payload_, (void *)buffer, data->length_);
      buffer += data->length_;
      if (*(uint16_t *)buffer != PACKET_END_IDENTIFIER) {
        error = END_OF_PACKET_MISSING;
        printf("End of packet id is wrong!\n");
      }
    }
  }
  return error;
}

AckPacket_t ReadAckPacket(const uint8_t *buffer) {
  AckPacket_t pack;
  if (buffer != NULL) {
    memcpy((void *)&pack, buffer, sizeof(pack));
  }
  return pack;
}

RejectPacket_t ReadRejectPacket(const uint8_t *buffer) {
  RejectPacket_t pack;
  if (buffer != NULL) {
    memcpy((void *)&pack, buffer, sizeof(pack));
  }
  return pack;
}