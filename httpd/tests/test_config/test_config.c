#include <stdio.h>
#include <string.h>

#include "../../src/config/config.h" 

void run_string_tests(void) {
  printf("--- lanclent : String Module Tests ---\n");

  const char *test1_data = "Hello";
  size_t test1_size = strlen(test1_data);
  struct string *str1 = string_create(test1_data, test1_size);

  if (str1 && str1->size == test1_size && strcmp(str1->data, test1_data) == 0) {
    printf("Test 1 (Create/Content): PASSED\n");
  } else {
    printf("Test 1 (Create/Content): FAILED\n");
  }

  const char test2_data[] = {'H', 'e', '\0', 'l', 'l', 'o'};
  size_t test2_size = 6;
  struct string *str2 = string_create(test2_data, test2_size);

  if (str2 && string_compare_n_str(str2, test2_data, 6) == 0) {
    printf("Test 2 (Null Byte Compare): PASSED\n");
  } else {
    printf("Test 2 (Null Byte Compare): FAILED (Expected: 0, Got: %d)\n",
           string_compare_n_str(str2, test2_data, 6));
  }

  const char *concat_data = " World";
  size_t concat_size = 6;
  string_concat_str(str1, concat_data, concat_size);

  if (str1 && str1->size == 11 && strcmp(str1->data, "Hello World") == 0) {
    printf("Test 3 (Concat): PASSED\n");
  } else {
    printf("Test 3 (Concat): FAILED\n");
  }

  string_destroy(str1);
  string_destroy(str2);
  printf("---------------------------------\n");
}
