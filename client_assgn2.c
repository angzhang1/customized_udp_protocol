/**
Client Assignment 2
Send subscriber to server to verify if it has paid for the access
**/

#include "common.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

bool running = true;

void intHandler(int dummy) {
  printf("Ctlr-C Captured!\n");
  running = false;
}

const uint8_t CLIENT_ID = 98;

int main(int argc, char **argv) {
  signal(SIGINT, &intHandler);

  FILE *fp = NULL;
  char *filename = "../good_data.txt";
  if (argc == 2) {
    filename = argv[argc - 1];
  }
  printf("Reading msg file: %s\n", filename);
  fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Open file %s failed\n", filename);
    return 0;
  }

  int fd;
  // create a udp socket
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("cannot create socket");
    return 0;
  }

  struct sockaddr_in server;
  memset((void *)&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(SERVER_IP);
  server.sin_port = htons(SERVER_PORT);

  // Set a timeout for 3 seconds for recvfrom
  struct timeval timeout;
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;

  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0) {
    perror("setsockopt failed\n");
    return 0;
  }

  uint8_t buff[MAX_MSG_LENGTH];
  uint8_t buff_out[MAX_MSG_LENGTH];

  DataPacket_t data_pack;
  data_pack.header_.client_id_ = CLIENT_ID;

  while (running &&
         fscanf(fp, "%hhu %hhu %s %hi", &data_pack.segment_, &data_pack.length_,
                data_pack.payload_, &data_pack.end_id_) == 4) {
    size_t data_len = WriteDataPacket(&data_pack, buff_out);
    // printf("data written: %s\n", buff_out + 7);
    // printf("Data packet len: %d\n", data_len);
    if (data_len == 0) {
      printf("Empty data!\n");
      break;
    }

    // Retry for ACK for at most 3 times
    const int RETRY_FOR_ACK = 3;
    uint32_t server_len = sizeof(server);
    int j = 0;
    for (j = 0; j <= RETRY_FOR_ACK; ++j) {
      int ret = sendto(fd, buff_out, data_len, 0, (struct sockaddr *)&server,
                       server_len);
      if (ret < 0) {
        perror("sendto error: \n");
        return 0;
      }
      ret = recvfrom(fd, buff, sizeof(buff), 0, (struct sockaddr *)&server,
                     &server_len);
      if (ret > 0) {
        break;
      } else if (ret < 0 && j < RETRY_FOR_ACK) {
        if (errno == EWOULDBLOCK) {
          printf(
              "No ACK message received, resending data, number of retry: %d\n",
              j + 1);
        } else {
          perror("Receving error: ");
        }
      }
    }

    if (j > RETRY_FOR_ACK) {
      fprintf(stderr, "Server does not respond\n");
      break;
    }

    AckPacket_t ack_packet;
    switch (ReadType(buff)) {
      case ACK:
        ack_packet = ReadAckPacket(buff);
        printf("Packet Acknownledged for segment %d \n",
               ack_packet.recv_segment_);
        break;
      case REJECT: {
        RejectPacket_t rej_pack = ReadRejectPacket(buff);
        printf("Packet rejected: ");
        switch (rej_pack.reject_code_) {
          case OUT_OF_SEQUENCE:
            printf("Out of sequence\n");
            break;
          case LENGTH_MISMATCH:
            printf("Length mismatch\n");
            break;
          case END_OF_PACKET_MISSING:
            printf("End of packet missing\n");
            break;
          case DUPLICATE_PACKET:
            printf("Duplicate packet\n");
            break;
          default:
            printf("Other reasons\n");
            break;
        }
        break;
      }
      case DATA:
      default:
        break;
    }
  }

  printf("Client exiting.\n");
  fclose(fp);
  close(fd);
  return 0;
}