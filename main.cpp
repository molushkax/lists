#include "list_funcs.h"
#include "hash.h"

int main () {
    struct info_about_list list = {};
    list_init (&list, 4);
    list_dump (&list);
    insert_after (LAST_ELEMENT, 15, &list);
    list_dump (&list);
    insert_after (LAST_ELEMENT, 16, &list);
    list_dump (&list);
    insert_after (LAST_ELEMENT, 17, &list);
    list_dump (&list);
    insert_after (LAST_ELEMENT, 18, &list);
    list_dump (&list);
    delete_after (1, &list);
    list_dump_dot (&list, "list.dot");
    insert_after (LAST_ELEMENT, 25, &list);
    list_dump (&list);
    insert_after (2, 17, &list);
    list_dump (&list);
    insert_after (NULL_ELEMENT, 14, &list);
    list_dump (&list);
    insert_after (4, 19, &list);
    insert_after (6, 20, &list);
    insert_after (NULL_ELEMENT, 21, &list);
    insert_after (10, 22, &list);
    insert_after (LAST_ELEMENT, 23, &list);
    insert_after (LAST_ELEMENT, 24, &list);
    insert_after (LAST_ELEMENT, 25, &list);
    list_dump (&list);
    list_destroy (&list);
    return 0;
}
