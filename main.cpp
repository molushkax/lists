#include "list_funcs.h"
#include "hash.h"

int main () {
    struct info_about_list list = {};

    FILE* log = list_log_open ("list_log.html");
    if (log == NULL) return 1;
    list_init (&list, 4);
    int action_id = 0;
    list_log_actions (&list, "list_init (size = 4)", log, &action_id);

    INSERT_AFTER(LAST_ELEMENT, 15, &list);
    INSERT_AFTER(LAST_ELEMENT, 16, &list);
    INSERT_AFTER(LAST_ELEMENT, 17, &list);
    INSERT_AFTER(LAST_ELEMENT, 18, &list);
    DELETE_AFTER(1, &list);
    list_dump_dot (&list, "list.dot");
    INSERT_AFTER(LAST_ELEMENT, 25, &list);
    INSERT_AFTER(2, 17, &list);
    INSERT_AFTER(NULL_ELEMENT, 14, &list);

    list_iterator_t head = get_head (&list);
    printf ("head: %zu\n", head);
    list_iterator_t tail = get_tail (&list);
    printf ("tail: %zu\n", tail);

    list_destroy (&list);
    list_log_close (log);
    return 0;
}
