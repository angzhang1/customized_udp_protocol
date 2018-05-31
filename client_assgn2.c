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
  char *filename = "../subscriber_input.txt";
  if (argc == 2) {
    filename = argv[argc - 1];
  }
  printf("Reading subscriber input file: %s\n", filename);
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

  uint8_t buff[14];
  uint8_t buff_out[14];

  VerificationPacket_t ver_pack;
  ver_pack.client_id_ = CLIENT_ID;
  ver_pack.start_id_ = PACKET_START_IDENTIFIER;
  ver_pack.status_ = ACC_PER;
  ver_pack.segment_ = 0;
  ver_pack.length_ = 5;
  ver_pack.end_id_ = PACKET_END_IDENTIFIER;

  uint8_t tech = 0;
  uint32_t sub_no = 0;
  while (running && fscanf(fp, "%u %hhu", &sub_no, &tech) == 2) {
    ver_pack.segment_++;
    ver_pack.subscriber_ = sub_no;
    ver_pack.technology_ = tech;
    size_t request_len = WriteVerificationPacket(&ver_pack, buff_out);
    // printf("data written: %s\n", buff_out + 7);
    // printf("Data packet len: %d\n", data_len);
    if (request_len == 0) {
      fprintf(stderr, "Empty data!\n");
      break;
    }
    printf("Sending request for %u, %dG\n", sub_no, tech);

    // Retry for ACK for at most 3 times
    const int RETRY_FOR_ACK = 3;
    uint32_t server_len = sizeof(server);
    int j = 0;
    for (j = 0; j <= RETRY_FOR_ACK; ++j) {
      int ret = sendto(fd, buff_out, request_len, 0, (struct sockaddr *)&server,
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
          printf("No message received, resending data, number of retry: %d\n",
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

    VerificationPacket_t response = ReadVerificationPacket(buff);
    printf("Response: ");
    switch (response.status_) {
      case NOT_PAID:
        printf("Subscriber %u not paid for %dG access\n", response.subscriber_,
               response.technology_);
        break;
      case NOT_EXIST:
        printf("Subscriber %u not exist in database\n", response.subscriber_);
        break;
      case ACCESS_OK:
        printf("Subscriber %u OK to access %dG\n", response.subscriber_,
               response.technology_);
        break;
      default:
        printf("Unknown response from server\n");
        break;
    }
    printf("----\n");
    sleep(1);
  }

  printf("Client exiting.\n");
  fclose(fp);
  close(fd);
  return 0;
}
