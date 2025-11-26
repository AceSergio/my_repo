#ifndef STRING_H
#define STRING_H

#include <stddef.h>

struct string {
  size_t size;
  char *data;
};

struct string *string_create(const char *src, size_t taille);

int string_compare_n_str(const struct string *s1, const char *s2, size_t n);

void string_concat_str(struct string *s, const char *ajout, size_t taille);

void string_destroy(struct string *s);

#endif /* ! STRING_H */
