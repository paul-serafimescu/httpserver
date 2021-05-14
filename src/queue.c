#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

/* creates queue and initializes head and tail with NULL */
queue_t *create_queue(void)
{
  queue_t *queue = malloc(sizeof(queue_t));
  queue->head = NULL;
  queue->tail = NULL;
  return queue;
}

/* returns 1 if queue is empty, 0 otherwise */
int is_empty(queue_t *queue)
{
  return queue->head == NULL;
}

/* creates a new node containing 'data' and adds it to the end of the queue */
int enqueue(queue_t *queue, int data)
{
  if (queue->head == NULL) {
    queue->head = malloc(sizeof(struct node));
    if (queue->head == NULL) return -1;
    queue->head->data = data;
    queue->head->next = NULL;
    queue->tail = queue->head;
  } else {
    struct node *new_node = malloc(sizeof(struct node));
    new_node->data = data;
    new_node->next = NULL;
    queue->tail->next = new_node;
    queue->tail = new_node;
  }

  return 0;
}

/* resolves the head of the queue, populates the value of 'data' with the value of the head */
int dequeue(queue_t *queue, int *data)
{
  if (is_empty(queue)) return -1;
  struct node *temp = queue->head->next;
  *data = queue->head->data;
  struct node *old_head = queue->head;
  queue->head = temp;
  free(old_head);

  if(queue->head == NULL) {
    queue->tail = NULL;
  }

  return 0;
}

/* deallocates queue */
void destroy_queue(queue_t *queue)
{
  if (queue == NULL) return;
  struct node *current = queue->head;
  struct node *temp = NULL;
  while (current != NULL) {
    temp = current->next;
    free(current);
    current = temp;
  }
  free(queue);
}

/* debugging function for queues */
void print_queue(queue_t *queue)
{
  struct node *cursor = queue->head;
  if (queue->head == NULL) {
    printf("empty queue\n");
    return;
  }
  do {
    printf("%d ", cursor->data);
    cursor = cursor->next;
  } while (cursor != NULL);
  printf("\n");
}
