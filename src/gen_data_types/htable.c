#include "htable.h"

static void* htable_passthrough_copy(void* elem) {
    return elem;
}

static void htable_passthrough_free(void* elem) {
    return;
}

static bool htable_passthrough_eq(const void* first, const void* second) {
    return (*(void**) first) - (*(void**) second) == 0 ? true : false;
}

static size_t htable_bucket_idx(htable_t* ht, void* key) {
    return ht->hash_fn(key, ht->seed) % ht->num_buckets;
}

static int htable_add_to_bucket(htable_t* ht, void* key, void* value,
        bool rehash) {
    size_t idx = htable_bucket_idx(ht, key);

    if (ht->buckets[idx].key == NULL) {
        key = ht->cbs.key_copy(key);
        if (key == NULL) {
            return -1;
        }
        if (value != NULL) {
            value = ht->cbs.value_copy(value);
            if (value == NULL) {
                return -1;
            }
        }
        ht->buckets[idx].key = key;
        ht->buckets[idx].value = value;
        if (!rehash) {
            ht->num_used++;
        }
    } else {
       htable_bucket_t* cur;
       htable_bucket_t* last;
       do {
            if (ht->keq(key, cur->key)) {
                if (cur->value != NULL) {
                    ht->cbs.value_free(cur->value);
                }
                if (!rehash && value != NULL) {
                    value = ht->cbs.value_copy(value);
                    if (value == NULL) {
                        return -1;
                    }
                }
                cur->value = value;
                last = NULL;
                break;
            }
            last = cur;
            cur = cur->next;
       } while (cur != NULL);

       if (last != NULL) {
            cur = calloc(1, sizeof(*cur->next));
            if (cur == NULL) {
                return -1;
            }
            if (!rehash) {
                key = ht->cbs.key_copy(key);
                if (key == NULL) {
                    return -1;
                }
                if (value != NULL) {
                    value = ht->cbs.value_copy(value);
                    if (value == NULL) {
                        return -1;
                    }
                }
            }
            cur->key = key;
            cur->value = value;
            last->next = cur;
            if (!rehash) {
                ht->num_used++;
            }
       }
    }
    return 0;
}

static int htable_rehash(htable_t* ht) {
   if (ht->num_used + 1 < (size_t) (ht->num_buckets*0.8)
           || ht->num_buckets >= 1 << 31) {
       return -1;
   }

   size_t num_buckets = ht->num_buckets;
   htable_bucket_t* buckets = ht->buckets;

   ht->num_buckets <<= 1;
   ht->buckets = calloc(ht->num_buckets, sizeof(*buckets));
    if (ht->buckets == NULL) {
        return -1;
    }

    htable_bucket_t* cur;
    htable_bucket_t* next;
   for (size_t i = 0; i < num_buckets; ++i) {
        if (buckets[i].key == NULL) {
            continue;
        }

        htable_add_to_bucket(ht, buckets[i].key, buckets[i].value, true);
        if (buckets[i].next != NULL) {
            cur = buckets[i].next;
            do {
                htable_add_to_bucket(ht, cur->key, cur->value, true);
                next = cur->next;
                free(cur);
                cur = next;
            } while (cur != NULL);
        }
   }
    free(buckets);
    return 0;
}

htable_t* create_htable(htable_hash fn, htable_keq keq, htable_cbs_t *cbs) {
    if (fn == NULL || keq == NULL) {
        return NULL;
    }

    htable_t* ht = calloc(1, sizeof(*ht));
    ht->hash_fn = fn;
    ht->keq = keq;

    ht->cbs.key_copy = cbs->key_copy == NULL ? 
        htable_passthrough_copy : cbs->key_copy;
    ht->cbs.key_free = cbs->key_free == NULL ?
        htable_passthrough_free : cbs->key_free;
    ht->cbs.value_eq = cbs->value_eq == NULL ? 
        htable_passthrough_eq : cbs->value_eq;
    ht->cbs.value_copy = cbs->value_copy == NULL ?
        htable_passthrough_copy : cbs->value_copy;
    ht->cbs.value_free = cbs->value_free == NULL ?
        htable_passthrough_free : cbs->value_free;

    ht->num_buckets = BUCKET_START;
    ht->buckets = calloc(BUCKET_START, sizeof(*ht->buckets));
    ht->num_used = 0;

    srand(time(NULL)); 
    ht->seed = rand();

    return ht;
}

int htable_destroy(htable_t* ht) {
    if (ht == NULL) {
        return -1;
    }
    htable_bucket_t* cur;
    htable_bucket_t* next;
    for (size_t i = 0; i < ht->num_buckets; ++i) {
        if (ht->buckets[i].key == NULL) {
            continue;
        }
        ht->cbs.key_free(ht->buckets[i].key);
        ht->cbs.value_free(ht->buckets[i].value);

        next = ht->buckets[i].next;
        while (next != NULL) {
            cur = next;
            ht->cbs.key_free(cur->key);
            ht->cbs.value_free(cur->value);
            next = cur->next;
            free(cur);
        }
    }
    free(ht->buckets);
    free(ht);

    return 0;
}
