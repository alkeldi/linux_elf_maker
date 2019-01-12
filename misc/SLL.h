#ifndef SLL_h
#define SLL_h

#include <stdlib.h>

/* 
 * SLL node structure.
 */
typedef struct SLLNode
{
  void *data;
  struct SLLNode *next;
} SLLNode;

/*
 * SLL info structure.
 */
typedef struct SLLInfo
{
  int size; /* number of nodes in the SLL */
  SLLNode *last;
} SLLInfo;

/* SLL Type*/
typedef SLLNode SLL;

/*
 * Singly linked list initializer.
 */
static SLL *SLL_init()
{

  /* SLL info keeper */
  SLLInfo *info = (SLLInfo *)malloc(sizeof(SLLInfo));

  /* SLL creation */
  SLL *node = (SLL *)malloc(sizeof(SLL));
  node->next = NULL;
  node->data = info;

  /* SLL info keeping */
  info->size = 0;
  info->last = node;

  return node;
}

/*
 * Add an element at the end of the singly linked list.
 * list: the main node of the singly linked list.
 * data: data to add to the list.
 * return value: the newly created node, which contains the data.
 */
static SLLNode *SLL_insert(SLL *sll, void *data)
{
  if (!sll)
    return NULL;

  /* get SLL info */
  SLLInfo *info = (SLLInfo *)sll->data;

  /* create the new node */
  SLLNode *node = (SLLNode *)malloc(sizeof(SLLNode));
  node->next = NULL;
  node->data = data;

  /* get last node */
  SLLNode *last = info->last;

  /* add the new node at the end */
  last->next = node;
  info->last = node;
  info->size++;

  return node;
}


/*
 * Remove the entire singly linked list.
 * list: the main node of the singly linked list.
 */
static void SLL_free(SLL *sll)
{
  if (!sll)
    return;

  /* free all the added nodes */
  SLLNode *iter = sll->next;
  while (iter)
  {
    SLLNode *next = iter->next;
    free(iter);
    iter = next;
  }

  /* free the main node and its info */
  SLLInfo *info = (SLLInfo *)sll->data;
  info->last = NULL;
  info->size = 0;
  free(info);
  info = NULL;
  free(sll);
  sll = NULL;
}

#endif