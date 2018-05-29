/**
Programming Assignment 1
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

int main(int argc, char **argv) {
  // Catch Ctlr-C
  signal(SIGINT, &intHandler);

  int fd;
  // create a udp socket
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("cannot create socket");
    return 0;
  }

  printf("created socket: descriptor = %d\n", fd);

  struct sockaddr_in server;
  memset((void *)&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(SERVER_IP);
  server.sin_port = htons(SERVER_PORT);

  if (bind(fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("bind failed");
    close(fd);
    return 0;
  }
  printf("bind complete. Port number = %d\n", ntohs(server.sin_port));

  uint8_t buff[MAX_MSG_LENGTH];
  uint8_t buff_out[MAX_MSG_LENGTH];

  struct sockaddr_in client;
  socklen_t len_client = sizeof(client);

  // Set a timeout for 3 seconds
  // otherwise recvfrom will block if no message received.
  struct timeval timeout;
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;

  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0) {
    perror("setsockopt failed\n");
    return 0;
  }

  // printf("data msg size: %lu\n", sizeof(RejectPacket_t));

  int expected_segment = 0;
  while (running) {
    int ret = recvfrom(fd, buff, sizeof(buff), 0, (struct sockaddr *)&client,
                       &len_client);
    if (ret < 0) {
      if (errno == EWOULDBLOCK) {
        printf("No message received, keep listening\n");
      } else {
        perror("Receving error: ");
      }
    } else {  // msg received

      DataPacket_t recv_data;
      int error_code = 0;
      switch (ReadType(buff)) {
        case DATA:
          error_code = ReadDataPacket(buff, ret, &recv_data);

          if (error_code == 0) {
            // No error, check sequence
            if (recv_data.segment_ == expected_segment) {
              error_code = DUPLICATE_PACKET;
              expected_segment--;
            } else if (recv_data.segment_ != expected_segment + 1) {
              error_code = OUT_OF_SEQUENCE;
              expected_segment--;
            } else {
              expected_segment++;
              printf("Good message, data: %s of length %d\n",
                     (char *)recv_data.payload_, recv_data.length_);
              // Good message, send ACK to client
              size_t ack_len = WriteAckPacket(recv_data.header_.client_id_,
                                              recv_data.segment_, buff_out);
              if (sendto(fd, buff_out, ack_len, 0, (struct sockaddr *)&client,
                         len_client) == -1) {
                perror("sendto error");
              }
              break;
            }
          }

          // Send reject packet if come here.
          printf("Reject sub code to send out: %x\n", error_code);
          expected_segment++;
          size_t rej_len =
              WriteRejectPacket(recv_data.header_.client_id_,
                                recv_data.segment_, error_code, buff_out);
          if (sendto(fd, buff_out, rej_len, 0, (struct sockaddr *)&client,
                     len_client) == -1) {
            perror("sendto error");
          }
          break;
        case ACK:
        case REJECT:
        case UNKNOWN:
        default:
          printf("Unknown or irrelevant type received!\n");
          break;
      }
    }
  }

  printf("Server exiting.\n");

  close(fd);

  return 0;
}
