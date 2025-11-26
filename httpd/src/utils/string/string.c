#include "string.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct string *string_create(const char *src, size_t taille) {
  struct string *s = malloc(sizeof(struct string));
  if (!s) {
    return NULL;
  }

  s->size = taille;
  s->data = NULL;

  if (taille > 0) {
    s->data = malloc(taille + 1);
    if (!s->data) {
      free(s);
      return NULL;
    }

    memcpy(s->data, src, taille);

    s->data[taille] = '\0';
  } else {
    s->data = NULL;
  }

  return s;
}

int string_compare_n_str(const struct string *s1, const char *s2, size_t n) {
  size_t l1 = s1->size;
  size_t len_comp = l1 < n ? l1 : n;

  int res = memcmp(s1->data, s2, len_comp);

  if (res != 0) {
    return res;
  }

  if (l1 < n && s2[l1] != 0) {
    return -1;
  }

  return 0;
}

void string_concat_str(struct string *s, const char *ajout, size_t taille) {
  if (!s || taille == 0) {
    return;
  }

  size_t anc_taille = s->size;
  size_t nouv_taille = anc_taille + taille;

  char *nouv_donnees = realloc(s->data, nouv_taille + 1);
  if (!nouv_donnees) {
    return;
  }

  memcpy(nouv_donnees + anc_taille, ajout, taille);

  s->data = nouv_donnees;
  s->size = nouv_taille;

  s->data[nouv_taille] = '\0';
}

void string_destroy(struct string *s) {
  if (!s) {
    return;
  }
  if (s->data) {
    free(s->data);
  }
  free(s);
}
