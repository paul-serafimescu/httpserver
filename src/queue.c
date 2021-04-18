#include <stdlib.h>
#include "queue.h"

queue_t *create_queue(void)
{
  queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
  queue->head = NULL;
  queue->tail = NULL;
  return queue;
}

int is_empty(queue_t *queue)
{
  return queue->head == NULL;
}

int enqueue(queue_t *queue, int data)
{
  if (queue->head == NULL) {
    queue->head = (struct node *)malloc(sizeof(struct node));
    if (queue->head == NULL) return -1;
    queue->head->data = data;
    queue->head->next = NULL;
    queue->tail = queue->head;
  } else {
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    new_node->data = data;
    new_node->next = NULL;
    queue->tail->next = new_node;
    queue->tail = new_node;
  }

  return 0;
}

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
