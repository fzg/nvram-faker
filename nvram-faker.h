#ifndef __NVRAM_FAKER_H__
#define __NVRAM_FAKER_H__

char *nvram_get(const char *key);
char **nvram_get_ex(const char *key, char **val, size_t len);
char *nvram_get_ex2(const char *key); // like get but with strdup

char *nvram_get_scanf(const char *key, const char *fmt, void **dest);
char *nvram_safe_get_ex(); // like get_ex
char *nvram_safe_get();  //?!
int nvram_safe_set(const char *key, char *val);
char *nvram_safe_set_printf(const char *key, const char *fmt, ...);



int nvram_match(const char *a, const char *b);
int nvram_invmatch(const char *a, const char *b);
int nvram_set(const char *key, char *value);
void nvram_unset(const char *key);

#endif /* __NVRAM_FAKER_H__ */
