#include "../include/string.h"

int strcmp(const char *str1, const char *str2) {
  int i = 0;
  while (str1[i] != '\0' && str2[i] != '\0') {
    if (str1[i] != str2[i]) {
      return 1;
    }
    i++;
  }
  if (str1[i] != str2[i]) {
    return 1;
  }
  return 0;
}

int strncmp(const char *str1, const char *str2, unsigned int num) {
  while (num > 0) {
    if (*str1 != *str2) {
      return (unsigned char)*str1 - (unsigned char)*str2;
    }
    if (*str1 == '\0') {
      return 0;
    }
    str1++;
    str2++;
    num--;
  }
  return 0;
}

float string_to_float(const char *str, int *out_length) {
  float result = 0.0f;
  float sign = 1.0f;
  int i = 0;

  if (str[i] == '-') {
    sign = -1.0f;
    i++;
  } else if (str[i] == '+') {
    i++;
  }

  while (str[i] >= '0' && str[i] <= '9') {
    result = result * 10.0f + (float)(str[i] - '0');
    i++;
  }

  if (str[i] == '.') {
    i++;
    float factor = 1.0f;
    while (str[i] >= '0' && str[i] <= '9') {
      factor *= 0.1f;
      result += (float)(str[i] - '0') * factor;
      i++;
    }
  }

  if (out_length) {
    *out_length = i;
  }

  return result * sign;
}
