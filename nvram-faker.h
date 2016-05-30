#ifndef __NVRAM_FAKER_H__
#define __NVRAM_FAKER_H__

char *nvram_get(const char *key);
char *nvram_get_ex(const char *key, char **val, size_t len);
char *nvram_get_scanf(const char *key, const char *fmt, void **dest);


int nvram_match(const char *a, const char *b);
void nvram_set(const char *key, char *value);
void nvram_unset(const char *key);

#endif /* __NVRAM_FAKER_H__ */
