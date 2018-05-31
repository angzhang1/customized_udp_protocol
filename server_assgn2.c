// server_assgn2.c
/*
@author Ang Zhang
@student id W1287478
*/

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

void intHandler(int dummy) { running = false; }

struct Subscriber_t {
  char phone_number_[10];
  int technology_;
  int paid_;
};

struct Node_t {
  struct Subscriber_t entry_;
  struct Node_t *next_;
};

typedef enum {
  PAID = 1,
  UNPAID = 0,
  NOT_FOUND = -1,
} Status_t;

// Search in the database for the phone no
// Return: 1: paid, 0: not paid, -1: not found (two cases)
Status_t IsPaid(uint32_t phone_no, int tech, const struct Node_t *table) {
  Status_t ret = NOT_FOUND;
  char phone_number[11];
  sprintf(phone_number, "%u", phone_no);

  while (table != NULL) {
    if (strcmp(table->entry_.phone_number_, phone_number) == 0) {
      if (tech == table->entry_.technology_) {
        ret = table->entry_.paid_;
        break;
      }
    }
    table = table->next_;
  }
  return ret;
}

int main(int argc, char **argv) {
  signal(SIGINT, &intHandler);

  FILE *fp = NULL;
  char *filename = "../Verification_Database.txt";
  if (argc == 2) {
    filename = argv[argc - 1];
  }
  printf("Reading data base file: %s\n", filename);
  fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "Open file %s failed\n", filename);
    return 0;
  }

  // Read the data and save to a linked list
  // The search will be linear complexity
  // We can improve this by using a hash table.
  // std::unordered_map can be used but it is out of the scope.
  // When the data is small, linear search is OK for this assignment.

  struct Node_t n;
  struct Node_t *dummy_head = &n;
  dummy_head->next_ = NULL;
  struct Node_t *cur = dummy_head;

  struct Subscriber_t sub;
  while (fscanf(fp, "%s %d %d", sub.phone_number_, &sub.technology_,
                &sub.paid_) == 3) {
    cur->next_ = (struct Node_t *)malloc(sizeof(struct Node_t));
    cur = cur->next_;
    cur->entry_ = sub;
    cur->next_ = NULL;
  }

  cur = dummy_head;
  while (cur->next_ != NULL) {
    cur = cur->next_;
    struct Subscriber_t *sub_entry = &cur->entry_;
    printf("Subscriber: %s, Technology: 0%d, Paid: %d\n",
           sub_entry->phone_number_, sub_entry->technology_, sub_entry->paid_);
  }

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

  uint8_t buff[14];
  uint8_t buff_out[14];

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

  struct Node_t *table = dummy_head->next_;

  while (running) {
    int ret = recvfrom(fd, buff, sizeof(buff), 0, (struct sockaddr *)&client,
                       &len_client);
    if (ret < 0) {
      if (errno == EWOULDBLOCK) {
        printf("Keep listening for messages\n");
      } else {
        perror("Receving error: ");
      }
    } else {  // msg received
      VerificationPacket_t pack = ReadVerificationPacket(buff);
      if (pack.status_ == ACC_PER) {
        Status_t query_res = IsPaid(pack.subscriber_, pack.technology_, table);

        printf("Request | Subscriber: %u, Technology: %dG, Status: ",
               pack.subscriber_, pack.technology_);

        if (query_res == NOT_FOUND) {
          pack.status_ = NOT_EXIST;
          printf("Not exist\n");
        } else if (query_res == PAID) {
          pack.status_ = ACCESS_OK;
          printf("Access OK!\n");
        } else if (query_res == UNPAID) {
          pack.status_ = NOT_PAID;
          printf("Not paid!\n");
        }

        size_t reply_len = WriteVerificationPacket(&pack, buff_out);
        if (sendto(fd, buff_out, reply_len, 0, (struct sockaddr *)&client,
                   len_client) == -1) {
          perror("sendto error");
        } else {
          printf("Response sent to client\n");
          printf("----\n");
        }
      } else {
        printf("Unknown request. Ignore this message\n");
      }
    }
  }

  // Delete linked list.
  cur = dummy_head->next_;

  while (cur != NULL) {
    struct Node_t *next = cur->next_;
    cur->next_ = NULL;
    free(cur);
    cur = next;
  }

  fclose(fp);
  close(fd);
  return 0;
}
