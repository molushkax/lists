#ifndef _LISTS_H_
#define _LISTS_H_

#define NDEBUG

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>

#define POISON_VALUE 777
#define NULL_POISON_VALUE 67

#define TWO_FOR_CANARIES 2
#define COEFFICIENT 2

typedef int list_elem_t;
typedef size_t list_iterator_t;

const size_t LIST_CANARY_L = (size_t)0xBEDABEDA;
const size_t LIST_CANARY_R = (size_t)0xBEDABEDAA;
const int DATA_CANARY_VALUE1 = 666;
const int DATA_CANARY_VALUE2 = 777;

#define NO_COLOR "\033[0m"
#define RED      "\033[1;31m"

#define LAST_ELEMENT list.number_of_elements
#define NULL_ELEMENT 0

enum list_error {
    LIST_OK = 0,
    LIST_NULL_PTR,
    LIST_BAD_LIST_CANARY_L_R,
    LIST_BAD_ARRAY_CANARY,
    LIST_BAD_HEAD,
    LIST_BAD_TAIL,
    LIST_BAD_SIZE,
    LIST_BAD_FREE_LIST,
    LIST_DELETE_NON_EXISTENT,
    LIST_ERROR_WITH_REALLOCATION,
    LIST_ERROR_WITH_ALLOCATION,
    LIST_INVALID_SIZE
};

struct node {
    list_elem_t data;
    list_iterator_t next;
    list_iterator_t prev;
};

struct info_about_list {
    size_t canary_l;
    node*  list_array;
    size_t number_of_elements;
    list_iterator_t first_free;
    size_t max_number_of_elements;
    uint32_t hash;
    size_t canary_r;
};

#define LIST_VERIFY(list) list_verify_func (list, __FUNCTION__, __LINE__)
#define LIST_PRINT_ERROR_CONTEXT(list) print_list_error_context_func (list, __FUNCTION__, __LINE__)
#define LIST_PRINT_CUSTOM_ERROR(list, error_code) print_list_custom_error_func (list, error_code, __FUNCTION__, __LINE__)

#define LIST_ASSERT(list) do { \
    int _n = list_verify_func (list, __FUNCTION__, __LINE__); \
    if (_n != 0) { \
        fprintf (stderr, RED "LIST_ASSERT failed (%d errors) at %s:%d\n" NO_COLOR, \
                _n, __FILE__, __LINE__); \
        abort(); \
    } \
} while (0)

#ifndef NDEBUG
    #define LIST_ASSERT_DBG(list) LIST_ASSERT(list)
#else
    #define LIST_ASSERT_DBG(list)
#endif

void list_set_canaries (info_about_list* list);
bool list_canaries_ok  (const info_about_list* list);
list_error list_init   (info_about_list* list, size_t size_of_list);
list_error insert_after_func  (list_iterator_t place_before, list_elem_t value, info_about_list* list);
void insert_after             (list_iterator_t place_before, list_elem_t value, info_about_list* list);
list_error realloc_up_array   (info_about_list* list);
void list_destroy             (info_about_list* list);
list_error delete_after_func  (list_iterator_t place_before, info_about_list* list);
void delete_after             (list_iterator_t place_before, info_about_list* list);
list_iterator_t iterator_next (info_about_list* list, list_iterator_t iterator);
list_iterator_t get_head      (info_about_list* list);
list_iterator_t get_tail      (info_about_list* list);

int  list_verify_func              (info_about_list* list, const char* func, int line);
void list_dump_in_log              (info_about_list* list, FILE* out);
void print_list_custom_error_func  (info_about_list* list, int error_code, const char* function_name, int line);
void print_list_error_context_func (info_about_list* list, const char* func, int line);
void list_dump_dot                 (info_about_list* list, const char* filename);

FILE* list_log_open    (const char* filename);
void  list_log_close   (FILE* log);
void  list_log_actions (info_about_list* list, const char* cmd, FILE* log, int* action_id);

#define INSERT_AFTER_LOG(place, value, list, log, action_id) do { \
    char buffer[128]; \
    snprintf (buffer, sizeof (buffer), "insert_after (place = %zu, value = %d)", \
              (size_t)place, (int)value); \
    insert_after (place, value, list); \
    list_log_actions (list, buffer, log, action_id); \
} while (0)

#define DELETE_AFTER_LOG(place, list, log, action_id) do { \
    char buffer[128]; \
    snprintf (buffer, sizeof (buffer), "delete_after (place = %zu)", \
              (size_t)place); \
    delete_after (place, list); \
    list_log_actions (list, buffer, log, action_id); \
} while (0)

#define INSERT_AFTER(place, value, list) INSERT_AFTER_LOG(place, value, list, log, &action_id)
#define DELETE_AFTER(place, list)        DELETE_AFTER_LOG(place, list, log, &action_id)

#endif
