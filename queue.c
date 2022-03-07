#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

#define MAX_STR_SIZE 1024
#define min(x, y) (x) < (y) ? (x) : (y)
/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    element_t *curr, *tmp;
    list_for_each_entry_safe (curr, tmp, l, list) {
        list_del(&curr->list);
        q_release_element(curr);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = malloc(sizeof(element_t));
    if (!ele)
        return false;
    list_add(&ele->list, head);

    char *str = malloc(sizeof(char) * strlen(s) + 1);
    if (!str)
        return false;
    ele->value = str;
    strncpy(ele->value, s, strlen(s) + 1);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ele = malloc(sizeof(element_t));
    if (!ele)
        return false;
    list_add_tail(&ele->list, head);

    char *str = malloc(sizeof(char) * strlen(s) + 1);
    if (!str)
        return false;
    ele->value = str;
    strncpy(ele->value, s, strlen(s) + 1);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *entry = list_first_entry(head, element_t, list);
    list_del(&entry->list);
    if (sp) {
        strncpy(sp, entry->value, bufsize);
    }
    return entry;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *entry = list_last_entry(head, element_t, list);
    list_del(&entry->list);
    if (sp) {
        strncpy(sp, entry->value, bufsize);
        /*
        int len = strlen(node->value) + 1;
        len = min(bufsize, len);
        strncpy(sp, node->value, len);
        sp[len - 1] = '\0';
        */
    }
    return entry;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    int num = 0;
    struct list_head *node;
    if (!head || list_empty(head))
        return 0;
    list_for_each (node, head)
        num++;
    return num;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;
    struct list_head **indir = &head;
    for (struct list_head *fast = head->next;
         fast != head && fast != head->prev; fast = fast->next->next)
        indir = &(*indir)->next;
    struct list_head *del = (*indir)->next;
    element_t *entry = list_entry(del, element_t, list);
    list_del(del);
    q_release_element(entry);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    element_t *n, *s;
    char *p = "";
    list_for_each_entry_safe (n, s, head, list) {
        if (!strncmp(n->value, p, MAX_STR_SIZE)) {
            list_del(&n->list);
            q_release_element(n);
        } else {
            p = n->value;
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head))
        return;
    struct list_head **indir = &head->next;
    struct list_head *tmp = NULL;
    while (*indir != head && (*indir)->next != head) {
        tmp = (*indir)->next;
        /* outer ptr exchange */
        (*indir)->next = tmp->next;
        tmp->prev = (*indir)->prev;
        /* inner ptr exchange */
        (*indir)->prev = tmp;
        tmp->next->prev = *indir;
        tmp->next = *indir;
        *indir = tmp;
        indir = &(*indir)->next->next;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) != 0)
        return;
    struct list_head *node = head;
    do {
        struct list_head *tmp = node->next;
        node->next = node->prev;
        node->prev = tmp;
        node = node->prev;
    } while (node != head);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */

struct list_head *mergeTwoLists(struct list_head *l1, struct list_head *l2)
{
    struct list_head *head = NULL, **node;
    struct list_head **ptr = &head;
    for (node = NULL; l1 && l2;) {
        if (strncmp(list_entry(l1, element_t, list)->value,
                    list_entry(l2, element_t, list)->value, MAX_STR_SIZE) < 0)
            node = &l1;
        else
            node = &l2;
        *ptr = *node;
        ptr = &(*ptr)->next;
        *node = (*node)->next;
    }
    *ptr = (struct list_head *) ((uintptr_t) l1 | (uintptr_t) l2);
    return head;
}

struct list_head *mergesort(struct list_head *head)
{
    if (!head || !head->next)
        return head;
    struct list_head *slow = head;
    for (struct list_head *fast = head->next; fast && fast->next;
         fast = fast->next->next)
        slow = slow->next;
    struct list_head *mid = slow->next;
    slow->next = NULL;

    struct list_head *left = mergesort(head), *right = mergesort(mid);
    return mergeTwoLists(left, right);
}

void q_sort(struct list_head *head)
{
    if (!head || head->next == head->prev) {
        return;
    }
    head->prev->next = NULL;
    head->next = mergesort(head->next);
    struct list_head *ptr = head;
    for (; ptr->next; ptr = ptr->next)
        ptr->next->prev = ptr;
    ptr->next = head;
    head->prev = ptr;
}
