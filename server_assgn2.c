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

// Search in the database for the phone no
// Return: 1: paid, 0: not paid, -1: not found (two cases)
int IsPaid(uint32_t phone_no, int tech, const struct Node_t *table) {
  int ret = -1;
  char phone_number[11];
  sprintf(phone_number, "%u", phone_no);

  while (table != NULL) {
    if (strcmp(table->entry_.phone_number_, phone_number) == 0) {
      if (tech == table->entry_.technology_) {
        ret =  table->entry_.paid_;
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

  printf("Is Paid: %d", IsPaid(4086808821, 3, dummy_head->next_));

  // Delete linked list.
  cur = dummy_head->next_;

  while (cur != NULL) {
    struct Node_t *next = cur->next_;
    cur->next_ = NULL;
    free(cur);
    cur = next;
  }

  fclose(fp);
  return 0;
}
