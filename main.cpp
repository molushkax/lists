#include "list_funcs.h"
#include "hash.h"

int main () {
    struct info_about_list list = {};

    FILE* log = list_log_open ("list_log.html");
    if (log == NULL) return 1;
    list_init (&list, 4);

    int action_id = 0;
    list_log_actions (&list, "list_init (size = 4)", log, &action_id);
    INSERT_AFTER(LAST_ELEMENT, 15, &list, log, &action_id);
    INSERT_AFTER(LAST_ELEMENT, 16, &list, log, &action_id);
    INSERT_AFTER(LAST_ELEMENT, 17, &list, log, &action_id);

    INSERT_AFTER(LAST_ELEMENT, 18, &list, log, &action_id);

    DELETE_AFTER(1, &list, log, &action_id);
    list_dump_dot (&list, "list.dot");
    INSERT_AFTER(LAST_ELEMENT, 25, &list, log, &action_id);

    INSERT_AFTER(2, 17, &list, log, &action_id);

    INSERT_AFTER(NULL_ELEMENT, 14, &list, log, &action_id);

    INSERT_AFTER(4, 19, &list, log, &action_id);
    INSERT_AFTER(6, 20, &list, log, &action_id);
    INSERT_AFTER(NULL_ELEMENT, 21, &list, log, &action_id);
    INSERT_AFTER(10, 22, &list, log, &action_id);
    INSERT_AFTER(LAST_ELEMENT, 23, &list, log, &action_id);
    INSERT_AFTER(LAST_ELEMENT, 24, &list, log, &action_id);
    INSERT_AFTER(LAST_ELEMENT, 25, &list, log, &action_id);

    list_destroy (&list);
    list_log_close (log);
    return 0;
}
