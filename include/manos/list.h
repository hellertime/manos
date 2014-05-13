#ifndef MANOS_LIST_H
#define MANOS_LIST_H

/**
 * Code adapted from linux kernel (include/linux/list.h)
 * Copyright belongs to linux kernel developers
 * License is GPLv2 with all linux kernel concessions
 * TODO: Update license info
 */

#include <manos/types.h>

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct ListHead name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
    (ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while(0)

/**
 * __list_add() - add new node to list between next and prev
 * @new:          the node to add
 * @prev:         the node previous to new
 * @next:         the node after new
 *
 * INTERNAL LIST ROUTINE! Consumers should look to use listAddAfter()
 */
static inline void __listAdd(ListHead* new, ListHead* prev, ListHead* next) {
    next->prev = new;
    new->next  = next;
    new->prev  = prev;
    prev->next = new;
}

/**
 * listAddAfter() - add a new node to the list
 * @new:            the node to add
 * @head:           the node to add after
 */
static inline void listAddAfter(ListHead* new, ListHead* head) {
    __listAdd(new, head, head->next);
}

/**
 * listAddBefore() - add a new node to the list
 * @new:             the node to add
 * @head:            the node to add before
 */
static inline void listAddBefore(ListHead* new, ListHead* head) {
    __listAdd(new, head->prev, head);
}

/**
 * __listUnlink() - unlink an entry from the list
 * @prev:           previous node
 * @next:           next node
 *
 * INTERNAL ONLY. Users would instead use listUnlink()
 */
static inline void __listUnlink(ListHead* prev, ListHead* next) {
    next->prev = prev;
    prev->next = next;
}

/**
 * listUnlink() - unlink the node from the list
 * @node:         the node to unlink
 */
static inline void listUnlink(ListHead* node) {
    __listUnlink(node->prev, node->next);
    node->prev = 0;
    node->next = 0;
}

/**
 * listUnlinkAndInit() - unlink a node an reinit it
 * @node:                the node to delete and reinit
 */
static inline void listUnlinkAndInit(ListHead* node) {
    __listUnlink(node->prev, node->next);
    INIT_LIST_HEAD(node);
}

/**
 * listIsEmpty() - test for emptiness
 * @head:          head of the list to check
 */
static inline int listIsEmpty(ListHead* head) {
    return head->next == head;
}

#define CONTAINER_OF(ptr, type, member) ({                  \
        const typeof(((type*)0)->member) *__mptr = (ptr);   \
        (type*)((char*)__mptr - offsetof(type,member));})

/**
 * LIST_FOR_EACH() - iterate forward over a list
 * @pos:             node used as a cursor
 * @head:            start of iteration
 */
#define LIST_FOR_EACH(pos, head) \
    for (pos = (head)->next; pos != (head); \
            pos = pos->next)

/**
 * LIST_EACH_FOR() - iterate backwards over a list
 * @pos:             cursor node
 * @head:            start of iteration
 */
#define LIST_EACH_FOR(pos, head) \
    for (pos = (head)->prev; pos != (head); \
            pos = pos->prev)

/**
 * LIST_FOR_EACH_SAFE() - iterate over a list, safe if node is removed
 * @pos:                  cursor node
 * @x:                    temp node
 * @head:                 start of iteration
 */
#define LIST_FOR_EACH_SAFE(pos, x, head) \
    for (pos = (head)->next, x = pos->next; pos != (head); \
            pos = x, x = pos->next)

/**
 * LIST_FOR_EACH_ENTRY() - iterator over a list by container
 * @pos:                   cursor of container type
 * @head:                  start of iteration
 * @member:                list member of pos struct
 */
#define LIST_FOR_EACH_ENTRY(pos, head, member)                      \
    for (pos = CONTAINER_OF((head)->next, typeof(*pos), member);    \
            &pos->member != (head);                                 \
            pos = CONTAINER_OF(pos->member.next, typeof(*pos), member))

/**
 * LIST_FOR_EACH_ENTRY_SAFE() - safe iterator over list by container
 * @pos:                        cursor of container type
 * @x:                          temp node
 * @head:                       start of iteration
 * @member:                     member of struct for list pointer
 */
#define LIST_FOR_EACH_ENTRY_SAFE(pos, x, head, member)                  \
    for (pos = CONTAINER_OF((head)->next, typeof(*pos), member),        \
            x = CONTAINER_OF(pos->member.next, typeof(*pos), member);   \
            &pos->member != (head);                                     \
            pos = x, x = CONTAINER_OF(x->member.next, typeof(*x), member))

#endif /* ! MANOS_LIST_H */
