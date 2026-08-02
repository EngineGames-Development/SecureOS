#ifndef STRING_H
#define STRING_H

int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, unsigned int num);
float string_to_float(const char *str, int *out_length);

#endif
