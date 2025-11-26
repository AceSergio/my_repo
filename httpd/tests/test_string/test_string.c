#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/string/string.h"

void test_create(void) {
  printf("[TEST] String Create... ");
  struct string *s = string_create("Hello", 5);
  assert(s != NULL);
  assert(s->size == 5);
  assert(memcmp(s->data, "Hello", 5) == 0);
  string_destroy(s);
  printf("OK\n");
}

void test_concat(void) {
  printf("[TEST] String Concat... ");
  struct string *s = string_create("Hello", 5);
  string_concat_str(s, " World", 6);
  assert(s->size == 11);
  assert(memcmp(s->data, "Hello World", 11) == 0);
  string_destroy(s);
  printf("OK\n");
}

void test_compare(void) {
  printf("[TEST] String Compare... ");
  struct string *s = string_create("Toto", 4);
  assert(string_compare_n_str(s, "Toto", 4) == 0);
  assert(string_compare_n_str(s, "Titi", 4) != 0);
  string_destroy(s);
  printf("OK\n");
}

int main(void) {
  printf("=== STRING SUITE ===\n");
  test_create();
  test_concat();
  test_compare();
  return 0;
}
