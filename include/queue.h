#ifndef QUEUE_H
#define QUEUE_H

struct node {
  int data;
  struct node *next;
};

typedef struct {
  struct node *head;
  struct node *tail;
} queue_t;

queue_t *create_queue(void);
int is_empty(queue_t *queue);
int enqueue(queue_t *queue, int data);
int dequeue(queue_t *queue, int *data);
void destroy_queue(queue_t *queue);
void print_queue(queue_t *queue);

#endif
