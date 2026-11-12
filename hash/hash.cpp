#include <stdint.h>
#include "list_funcs.h"
#include "hash.h"

// dgb2 хэш функция
// https://habr.com/ru/companies/otus/articles/541378/?ysclid=muvuuczsvq760208665
// хэшируется всё, что определяет состояние списка
// поле list->hash НЕ хэшируется — иначе функция зависела бы сама от себя.
uint32_t dgb2_hash (const info_about_list* list) {
    assert (stk != NULL);

    uint32_t h = 5381u;

    h = hash_u64 (h, (uint64_t)list->canary_l);
    h = hash_u64 (h, (uint64_t)list->canary_r);

    h = hash_u64 (h, (uint64_t)list->number_of_elements);
    h = hash_u64 (h, (uint64_t)list->max_number_of_elements);

    h = hash_u64 (h, (uint64_t)list->first_free);

    if (list->list_array != NULL && list->max_number_of_elements > 0) {
        uint64_t left_canary = 0;
        memcpy (&left_canary, &list->list_array[-1].data, sizeof (list_elem_t));
        h = hash_u64 (h, left_canary);

        for (size_t i = 0; i < list->max_number_of_elements; i++) {
            uint64_t value_bits = 0;
            memcpy (&value_bits, &list->list_array[i], sizeof (list_elem_t));
            h = hash_u64 (h, value_bits);
        }

        uint64_t right_canary = 0;
        memcpy (&right_canary, &list->list_array[list->max_number_of_elements].data, sizeof (list_elem_t));
        h = hash_u64 (h, right_canary);
    }

    return h;
}

// замешивает один байт в текущее значение хэша: h = h * 33 + byte.
uint32_t hash_one_byte (uint32_t h, uint8_t byte) {
    return h * 33 + byte;
}

// замешивает 64-битное число в хэш, идя по его байтам.
uint32_t hash_u64 (uint32_t h, uint64_t value) {
    for (size_t i = 0; i < sizeof (value); i++) {
        h = hash_one_byte (h, (uint8_t)(value >> (8 * i))); //побитовый сдвиг вправо
    }

    return h;
}

// Пересчитывает хэш и записывает его в list->hash.
void list_update_hash (info_about_list* list) {
    if (list == NULL) return;

    list->hash = dgb2_hash (list);
}
