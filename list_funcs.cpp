#include "list_funcs.h"
#include "hash.h"

void list_set_canaries (info_about_list* list) {
    assert (list != NULL);

    list->canary_l = LIST_CANARY_L;
    list->canary_r = LIST_CANARY_R;
}

bool list_canaries_ok (const info_about_list* list) {
    if (list == NULL) return false;
    return list->canary_l == LIST_CANARY_L && list->canary_r == LIST_CANARY_R;
}

list_error list_init (info_about_list* list, size_t size_of_list) {
    assert (list != NULL);
    if (size_of_list <= 0) return LIST_INVALID_SIZE;

    list_set_canaries(list);

    node* raw = (node*)calloc (size_of_list + TWO_FOR_CANARIES, sizeof (node));
    if (raw == NULL) return LIST_ERROR_WITH_ALLOCATION;

    list->list_array = raw + 1;

    list->list_array[-1].data           = DATA_CANARY_VALUE1;
    list->list_array[size_of_list].data = DATA_CANARY_VALUE2;


    list->number_of_elements = 0;
    list->max_number_of_elements = size_of_list;

    list->list_array[0].next = 0;
    list->list_array[0].prev = 0;
    list->list_array[0].data = NULL_POISON_VALUE;

    for (list_iterator_t i = 1; i < size_of_list; i++) {
        list->list_array[i].next = (i == size_of_list - 1) ? POISON_VALUE : i + 1;
        list->list_array[i].prev = POISON_VALUE;
        list->list_array[i].data = 0;
    }

    list->first_free = (size_of_list > 1) ? 1 : POISON_VALUE;

    list_update_hash(list);

    LIST_ASSERT(list);
    return LIST_OK;
}

list_error insert_after_func (list_iterator_t place_before, list_elem_t value, info_about_list* list) {
    LIST_ASSERT(list);

    if (list->first_free == POISON_VALUE) {
        if (realloc_up_array (list) != LIST_OK) return LIST_ERROR_WITH_REALLOCATION;
    }

    list_iterator_t index_of_next_element = 0;
    for (list_iterator_t i = 0; i < place_before; i++) {
        index_of_next_element = list->list_array[index_of_next_element].next;
    }

    list_iterator_t new_index = list->first_free;
    list->first_free = list->list_array[new_index].next;

    list_iterator_t next_element = list->list_array[index_of_next_element].next;

    list->list_array[new_index].prev = index_of_next_element;
    list->list_array[new_index].data = value;
    list->list_array[new_index].next = next_element;

    list->list_array[index_of_next_element].next = new_index;
    list->list_array[next_element].prev = new_index;

    list->number_of_elements++;

    list_update_hash(list);

    LIST_ASSERT_DBG(list);
    return LIST_OK;
}

void insert_after (list_iterator_t place_before, list_elem_t value, info_about_list* list) {
    LIST_ASSERT(list);

    list_error err = insert_after_func (place_before, value, list);

    if (err != LIST_OK) LIST_VERIFY(list);
}

list_error realloc_up_array (info_about_list* list) {
    LIST_ASSERT(list);

    size_t old_max = list->max_number_of_elements;
    size_t new_max = old_max * COEFFICIENT;
    if (new_max <= old_max) new_max = old_max + 1;

    node* raw = list->list_array - 1;
    node* new_raw = (node*)realloc (raw, (new_max + TWO_FOR_CANARIES) * sizeof (node));

    if (new_raw == NULL) return LIST_ERROR_WITH_REALLOCATION;

    list->list_array = new_raw + 1;

    list->list_array[new_max].data = DATA_CANARY_VALUE2;

    list_iterator_t old_first_free = list->first_free;
    for (list_iterator_t i = old_max; i < new_max; i++) {
        list->list_array[i].prev = POISON_VALUE;
        list->list_array[i].data = 0;
        list->list_array[i].next = (i == new_max - 1) ? old_first_free : i + 1;
    }

    list->first_free = old_max;
    list->max_number_of_elements = new_max;
    printf ("%zu number of elements\n", list->number_of_elements);

    list_update_hash(list);

    LIST_ASSERT_DBG(list);
    return LIST_OK;
}

list_error delete_after_func (list_iterator_t place_before, info_about_list* list) {
    LIST_ASSERT(list);

    if (place_before > list->max_number_of_elements) {
        fprintf (stderr, RED "\nERROR: in list you have less elements\n" NO_COLOR);
        return LIST_DELETE_NON_EXISTENT;
    }

    list_iterator_t index_of_next_element = 0;
    for (list_iterator_t i = 0; i < place_before; i++) {
        index_of_next_element = list->list_array[index_of_next_element].next;
    }

    list_iterator_t delete_element = list->list_array[index_of_next_element].next;
    if (delete_element == 0) {
        fprintf (stderr, RED "\nERROR: nothing to remove after this element\n" NO_COLOR);
        return LIST_DELETE_NON_EXISTENT;
    }

    list_iterator_t after_delete_element = list->list_array[delete_element].next;
    list->list_array[index_of_next_element].next = after_delete_element;
    list->list_array[after_delete_element].prev = index_of_next_element;

    list->list_array[delete_element].data = 0;
    list->list_array[delete_element].prev = POISON_VALUE;
    list->list_array[delete_element].next = list->first_free;
    list->first_free                      = delete_element;

    list->number_of_elements--;

    list_update_hash(list);

    LIST_ASSERT_DBG(list);
    return LIST_OK;
}

void delete_after (list_iterator_t place_before, info_about_list* list) {
    LIST_ASSERT(list);

    list_error err = delete_after_func (place_before, list);

    if (err != LIST_OK) LIST_VERIFY(list);
}

int list_verify_func (info_about_list* list, const char* func, int line) {
    int n = 0;

    if (list == NULL) {
        if (strstr (func, "test_list") == NULL) {
            fprintf (stderr, RED "\nERROR: list pointer is NULL\n" NO_COLOR);
        }
        return 1;
    }

    if (list->canary_l != LIST_CANARY_L) {
        if (strstr (func, "test_list") == NULL) {
            fprintf (stderr, RED "\nERROR: left struct canary damaged. \n"
                     "Expected 0x%zX, got 0x%zX\n" NO_COLOR,
                     LIST_CANARY_L, list->canary_l);
            print_list_error_context_func (list, func, line);
        }
        n++;
    }

    if (list->canary_r != LIST_CANARY_R) {
        if (strstr (func, "test_list") == NULL) {
            fprintf (stderr, RED "\nERROR: right struct canary damaged. \n"
                     "Expected 0x%zX, got 0x%zX\n" NO_COLOR,
                     LIST_CANARY_R, list->canary_r);
            print_list_error_context_func (list, func, line);
        }
        n++;
    }

    if (list->list_array == NULL) {
        if (strstr (func, "test_list") == NULL) {
            fprintf (stderr, RED "\nERROR: list_array is NULL\n" NO_COLOR);
            print_list_error_context_func (list, func, line);
        }
        return n + 1;
    } //TODO дописать

    if (list->list_array[-1].data != DATA_CANARY_VALUE1) {
        if (strstr (func, "test_list") == NULL) {
            fprintf (stderr, RED "\nERROR: right list canary damaged. \n"
                     "Expected %d, got %d\n" NO_COLOR,
                     DATA_CANARY_VALUE1, list->list_array[-1].data);
            print_list_error_context_func (list, func, line);
        }
        n++;
    }

    if (list->list_array[list->max_number_of_elements].data != DATA_CANARY_VALUE2) {
        if (strstr (func, "test_list") == NULL) {
            fprintf (stderr, RED "\nERROR: left list canary damaged. \n"
                     "Expected %d, got %d\n" NO_COLOR,
                     DATA_CANARY_VALUE2, list->list_array[list->max_number_of_elements].data);
            print_list_error_context_func (list, func, line);
        }
        n++;
    }

    if (list->list_array[0].data != NULL_POISON_VALUE) {
        if (strstr (func, "test_list") == NULL) {
            fprintf (stderr, RED "\nERROR: null element damaged. \n"
                     "Expected %d in data, got %d\n" NO_COLOR,
                     NULL_POISON_VALUE, list->list_array[0].data);
            print_list_error_context_func (list, func, line);
        }
        n++;
    }

    list_iterator_t next = 0;
    for (size_t i = 0; i < list->number_of_elements; i++) {
        next = list->list_array[next].next;
    }

    if (next != list->list_array[0].prev) {
        if (strstr (func, "test_list") == NULL) {
            fprintf (stderr, RED "\nERROR: tail in null element != real tail \n"
                     "Expected %zu iterator, got %zu\n" NO_COLOR,
                     list->list_array[0].prev, next);
            print_list_error_context_func (list, func, line);
        }
        n++;
    }

    uint32_t expected_hash = dgb2_hash (list);
    if (list->hash != expected_hash) {
        if (strstr (func, "test_list") == NULL) {
            fprintf (stderr, RED "ERROR: list hash mismatch.\n" NO_COLOR
                    "Expected 0x%08X, got 0x%08X\n"
                    "File: %s, function: %s, line %d\n\n",
                    expected_hash, list->hash, __FILE__, func, line);
            print_list_error_context_func (list, func, line);
        }

        n++;
    }

    return 0;
}

void list_dump (info_about_list* list) {
    assert (list != NULL);

    printf ("---------------------------------\n");
    printf ("head: %zu | tail: %zu | first_free: %zu\n", list->list_array[0].next, list->list_array[0].prev, list->first_free);
    printf ("number of elements in list: %zu\n", list->number_of_elements);
    printf ("---------------------------------\n");

    #ifndef NDEBUG
        printf ("left  canary of struct with info about list: 0x%zX\n", list->canary_l);
        printf ("right canary of struct with info about list: 0x%zX\n", list->canary_r);
        printf ("---------------------------------\n");
        printf ("left  canary of list: 0x%zX\n", list->list_array[-1].data);
        printf ("right canary of list: 0x%zX\n", list->list_array[list->max_number_of_elements].data);
        printf ("---------------------------------\n");
    #endif

    printf ("index |  data  |  next  |  prev \n");
    printf ("---------------------------------\n");

    for (list_iterator_t i = 0; i < list->max_number_of_elements; i++) {
        printf ("%5zu | %6d | %6zu | %6zu\n", i, (list->list_array[i]).data, (list->list_array[i]).next, (list->list_array[i]).prev);
    }
    printf ("\n======================================\n");
}

void list_dump_dot(info_about_list* list, const char* filename) {
    if (!list || !filename) return;

    FILE* f = fopen(filename, "w");
    if (!f) return;

    fprintf(f, "digraph list {\n");
    fprintf(f, "  rankdir=TB;\n");
    fprintf(f, "  splines=ortho;\n");
    fprintf(f, "  nodesep=0.6;\n");
    fprintf(f, "  ranksep=1.0;\n");
    fprintf(f, "  node [shape=none, fontname=\"Helvetica\", fontsize=11];\n");
    fprintf(f, "  edge [fontname=\"Helvetica\", fontsize=10, arrowsize=0.8];\n\n");

    // узлы все
    for (size_t i = 0; i < list->max_number_of_elements; i++) {
        node* n = &list->list_array[i];
        bool is_free = (n->prev == POISON_VALUE && n->data == 0);
        const char* fillcolor = is_free ? "#F5F5F5" : "#FFEBEE";
        const char* bordercolor = is_free ? "#BDBDBD" : "#B71C1C";

        fprintf(f, "  n%zu [label=<\n", i);
        fprintf(f, "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" BGCOLOR=\"%s\" COLOR=\"%s\">\n", fillcolor, bordercolor);
        fprintf(f, "      <TR><TD PORT=\"idx\" BGCOLOR=\"#E3E3E3\" WIDTH=\"40\"><B>%zu</B></TD></TR>\n", i);
        fprintf(f, "      <TR><TD PORT=\"data\" WIDTH=\"60\">data: %d</TD></TR>\n", n->data);
        fprintf(f, "      <TR><TD PORT=\"next\" ALIGN=\"LEFT\" WIDTH=\"60\">next: %zu</TD></TR>\n", n->next);
        fprintf(f, "      <TR><TD PORT=\"prev\" ALIGN=\"RIGHT\" WIDTH=\"60\">prev: %zu</TD></TR>\n", n->prev);
        fprintf(f, "    </TABLE>\n  >];\n\n");
    }

    // свободные на месте свободных
    for (size_t i = 0; i < list->max_number_of_elements; i++) {
        node* n = &list->list_array[i];
        bool is_free = (n->prev == POISON_VALUE && n->data == 0);
        if (is_free) {
            fprintf(f, "  phantom%zu [shape=point, style=invis, width=0, height=0];\n", i);
        }
    }

    // по рядам расставляем
    fprintf(f, "  { rank=same; ");
    for (size_t i = 0; i < list->max_number_of_elements; i++) {
        node* n = &list->list_array[i];
        bool is_free = (n->prev == POISON_VALUE && n->data == 0);
        if (!is_free) fprintf(f, "n%zu; ", i);
        else fprintf(f, "phantom%zu; ", i);
    }
    fprintf(f, "}\n");

    fprintf(f, "  { rank=same; ");
    for (size_t i = 0; i < list->max_number_of_elements; i++) {
        node* n = &list->list_array[i];
        bool is_free = (n->prev == POISON_VALUE && n->data == 0);
        if (is_free) fprintf(f, "n%zu; ", i);
    }
    fprintf(f, "}\n\n");

    // свободные с местами связываем
    for (size_t i = 0; i < list->max_number_of_elements; i++) {
        node* n = &list->list_array[i];
        bool is_free = (n->prev == POISON_VALUE && n->data == 0);
        if (is_free) {
            fprintf(f, "  phantom%zu -> n%zu [style=invis, weight=100];\n", i, i);
        }
    }

    // невидимый скелет
    for (size_t i = 0; i < list->max_number_of_elements - 1; i++) {
        node* n1 = &list->list_array[i];
        node* n2 = &list->list_array[i+1];
        bool free1 = (n1->prev == POISON_VALUE && n1->data == 0);
        bool free2 = (n2->prev == POISON_VALUE && n2->data == 0);
        const char* node1 = free1 ? "phantom" : "n";
        const char* node2 = free2 ? "phantom" : "n";
        fprintf(f, "  %s%zu -> %s%zu [style=invis, weight=10];\n", node1, i, node2, i + 1);
    }

    // некст красные
    // справа (:e) налево (:w).
    for (size_t i = 0; i < list->max_number_of_elements; i++) {
        node* n = &list->list_array[i];
        bool is_free = (n->prev == POISON_VALUE && n->data == 0);

        if (!is_free && n->next < list->max_number_of_elements && n->next != POISON_VALUE) {
            fprintf(f, "  n%zu:next:e -> n%zu:data:w [style=bold, color=\"#D32F2F\", penwidth=2, constraint=false];\n", i, n->next);
        }
    }

    // синие прев
    // слева (:w) направо (:e).
    for (size_t i = 0; i < list->max_number_of_elements; i++) {
        node* n = &list->list_array[i];
        bool is_free = (n->prev == POISON_VALUE && n->data == 0);

        if (!is_free && n->prev < list->max_number_of_elements && n->prev != POISON_VALUE) {
            fprintf(f, "  n%zu:prev:w -> n%zu:data:e [style=bold, color=\"#1976D2\", penwidth=2, constraint=false];\n", i, n->prev);
        }
    }

    // серые свободные
    for (size_t i = 0; i < list->max_number_of_elements; i++) {
        node* n = &list->list_array[i];
        bool is_free = (n->prev == POISON_VALUE && n->data == 0);

        if (is_free && n->next < list->max_number_of_elements && n->next != POISON_VALUE) {
            fprintf(f, "  n%zu:next:e -> n%zu:data:w [style=dashed, color=\"#757575\", penwidth=1.5, constraint=false];\n", i, n->next);
        }
    }

    // 9. метки
    fprintf(f, "\n  head_label [shape=box, style=\"filled\", fillcolor=\"#FFCDD2\", color=\"#D32F2F\", label=\"head\"];\n");
    fprintf(f, "  tail_label [shape=box, style=\"filled\", fillcolor=\"#BBDEFB\", color=\"#1976D2\", label=\"tail\"];\n");
    fprintf(f, "  free_label [shape=box, style=\"filled\", fillcolor=\"#E0E0E0\", color=\"#757575\", label=\"free\"];\n");

    if (list->list_array[0].next < list->max_number_of_elements) {
        fprintf(f, "  head_label -> n%zu:idx:w [style=dotted, color=\"#D32F2F\", constraint=false];\n", list->list_array[0].next);
    }
    if (list->list_array[0].prev < list->max_number_of_elements) {
        fprintf(f, "  tail_label -> n%zu:idx:e [style=dotted, color=\"#1976D2\", constraint=false];\n", list->list_array[0].prev);
    }
    if (list->first_free < list->max_number_of_elements) {
        fprintf(f, "  free_label -> n%zu:idx:w [style=dotted, color=\"#757575\", constraint=false];\n", list->first_free);
    }

    fprintf(f, "}\n");
    fclose(f);
}

void print_list_error_context_func (info_about_list* list, const char* func, int line) {
    fprintf(stderr,
            "Error context:\n"
            "  file:       %s\n"
            "  function:   %s\n"
            "  line:       %d\n"
            "  struct:     info_about_list at %p\n",
            __FILE__, func, line, (void*)list);

    if (list != NULL) {
        fprintf(stderr,
            "  list_array: %p\n"
            "  size:       %zu\n"
            "  max:        %zu\n"
            "  first_free: %zu\n"
            "  canary L:   0x%zX\n"
            "  canary R:   0x%zX\n\n",
            (void*)list->list_array,
            list->number_of_elements,
            list->max_number_of_elements,
            list->first_free,
            list->canary_l,
            list->canary_r);
    } else {
        fprintf(stderr, "\n");
    }
}

void print_list_custom_error_func (info_about_list* list, int error_code,
                                   const char* function_name, int line) {
    fprintf(stderr, RED "ERROR %d in list\n" NO_COLOR, error_code);
    print_list_error_context_func(list, function_name, line);
}

void list_destroy (info_about_list* list) {
    assert (list != NULL);

    free (list->list_array);
    list->number_of_elements = 0;
    list->first_free = POISON_VALUE;
    list->max_number_of_elements = 0;
    list->hash = 0;
}
