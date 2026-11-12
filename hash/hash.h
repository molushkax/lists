#ifndef HASH_H
    #define HASH_H

uint32_t   hash_one_byte     (uint32_t h, uint8_t byte);
uint32_t   hash_u64          (uint32_t h, uint64_t value);
uint32_t   dgb2_hash         (const info_about_list* list);
void       list_update_hash  (info_about_list* list);

#endif
