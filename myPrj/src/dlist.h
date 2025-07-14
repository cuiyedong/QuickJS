/*
 * =====================================================================================
 *
 *  Copyright (c) 2020. Huami Ltd, unpublished work. This computer program includes
 *  Confidential, Proprietary Information and is a Trade Secret of Huami Ltd.
 *  All use, disclosure, and/or reproduction is prohibited unless authorized in writing.
 *  All Rights Reserved.
 *
 *  Author:  xlbao@huami.com
 *
 * =====================================================================================
 */

#ifndef LIST_H_
#define LIST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdlib.h>

    typedef struct list_head
    {
        struct list_head *next, *prev;
    } list_head_t;

#define LIST_HEAD_INIT(el) \
    {                      \
        &(el), &(el)       \
    }

#define INIT_LIST_HEAD(p) \
    do                    \
    {                     \
        (p)->next = (p);  \
        (p)->prev = (p);  \
    } while (0)

#ifndef offset_of
    #define offset_of(type, member) ((size_t) & ((type *)0)->member)
#endif
#ifndef container_of
    #define container_of(ptr, type, member) (type *)((char *)ptr - offset_of(type, member));
#endif

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head)                   \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

#define list_for_each_prev(el, head) \
    for (el = (head)->prev; el != (head); el = el->prev)

#define list_for_each_prev_safe(el, el1, head)            \
    for (el = (head)->prev, el1 = el->prev; el != (head); \
         el = el1, el1 = el->prev)

    static inline void init_list_head(struct list_head *head)
    {
        head->prev = head;
        head->next = head;
    }

    static inline void __list_add(struct list_head *el,
                                  struct list_head *prev,
                                  struct list_head *next)
    {
        next->prev = el;
        el->next = next;
        el->prev = prev;
        prev->next = el;
    }

    static inline void list_add(struct list_head *el, struct list_head *head)
    {
        __list_add(el, head, head->next);
    }

    static inline void list_add_tail(struct list_head *el, struct list_head *head)
    {
        __list_add(el, head->prev, head);
    }

    static inline void list_del(struct list_head *item)
    {
        item->prev->next = item->next;
        item->next->prev = item->prev;
        item->prev = item;
        item->next = item;
    }

    static inline int list_empty(struct list_head *item)
    {
        return item->next == item;
    }

    static inline struct list_head *list_pop(struct list_head *head)
    {
        if (head->next == head)
            return NULL;

        struct list_head *p = head->next;
        list_del(head->next);
        return p;
    }

#ifdef __cplusplus
}
#endif
#endif /* LIST_H_ */
