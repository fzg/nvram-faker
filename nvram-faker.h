#ifndef __NVRAM_FAKER_H__
#define __NVRAM_FAKER_H__


// GET
char *nvram_get(const char *key);
char *nvram_get_ex(const char *key, char *val, size_t len);
char *nvram_get_ex2(const char *key); // like get but with strdup
int  nvram_get_scanf(const char *key, const char *fmt, ...);
char *nvram_safe_get();  //?!
char *nvram_safe_get_ex(); // like get_ex


// SET
int nvram_set(const char *key, char *value);
int nvram_set_printf(const char *key, const char *fmt, ...); // todo
int nvram_safe_set(const char *key, char *val);
int nvram_safe_set_printf(const char *key, const char *fmt, ...);

void nvram_unset(const char *key);

//MATCH
int nvram_match(const char *a, const char *b);
int nvram_invmatch(const char *a, const char *b);

//OTHER
int nvram_commit(int ); //todo
void ct_debuglog(int x, int y, char *fmt, ...);
void ct_syslog(int x, int y, char *fmt, ...);



#endif /* __NVRAM_FAKER_H__ */
