#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "nvram-faker.h"
//include before ini.h to override ini.h defaults
#include "nvram-faker-internal.h"
#include "ini.h"

#define RED_ON "\033[22;31m"
#define RED_OFF "\033[22;00m"
#define DEFAULT_KV_PAIR_LEN 1024

static int kv_count=0;
static int key_value_pair_len=DEFAULT_KV_PAIR_LEN;
static char **key_value_pairs=NULL;
static int is_value_ro(const char *key, char *v);
inline static int find_key(const char *key);


#define DIE(x) {DEBUG_PRINTF(x);return 0;}
#define SIE(x) {DEBUG_PRINTF(x);return;}

#include "nvram-faker-internal.c"


/*
   mmm  mmmmmmmmmmmmmmmmmmmm mmmmmm mmmmm   mmmm
 m"   " #        #      #    #      #   "# #"   "
 #   mm #mmmmm   #      #    #mmmmm #mmmm" "#mmm
 #    # #        #      #    #      #   "m     "#
  "mmm" #mmmmm   #      #    #mmmmm #    " "mmm#"

*/

char *nvram_get_ex2(const char *key) {
  LOG_PRINTF("get_ex2\t");
  return nvram_get(key);
}

char **nvram_get_ex(const char *key, char **val, size_t len) {
  LOG_PRINTF("get_ex\t");
  static char *ptr;
  ptr = nvram_get(key);
  *val = ptr;
  return &ptr;
}

int nvram_get_scanf(const char *key, const char *fmt, ...) {
  LOG_PRINTF("get_scanf(%s, %s)\t", key, fmt);
  va_list args;
  va_start(args, fmt);

  int len;
  char *it = nvram_get(key);
  if (!it) {
    LOG_PRINTF("non existing %s!\n", key);
    it = "0";
  }

  len = vsscanf(it, fmt, args);
  va_end(args);
  return len;
}

// TODO fix because apparently libnvram doesnt strdup for simple get (but for get_ex_2)
// aweful memory leak but don't care for now
char *nvram_get(const char *key) {
  LOG_PRINTF("get(%s)\t", key);
  int i, found = 0;
  if ((i = find_key(key)) > -1) {
    LOG_PRINTF("%s == %s\n",key,key_value_pairs[i+1]);
    found = !(!(key_value_pairs[i+1]));
    return found? strdup(key_value_pairs[i+1]) : NULL;
  }
  LOG_PRINTF( RED_ON"%s := Unknown\n"RED_OFF,key);
  return NULL;
}


/*
  mmmm  mmmmmmmmmmmmmmmmmmmm mmmmmm mmmmm   mmmm
 #"   " #        #      #    #      #   "# #"   "
 "#mmm  #mmmmm   #      #    #mmmmm #mmmm" "#mmm
     "# #        #      #    #      #   "m     "#
 "mmm#" #mmmmm   #      #    #mmmmm #    " "mmm#"

*/

int nvram_safe_set(const char *key, char *val) {
  LOG_PRINTF("safe_set(%s,%s)\t", key, val);
  if (!val || (val && !key)) return -1;
  else return nvram_set(key, val);
}

int nvram_set(const char *key, char *val) {
  LOG_PRINTF("set(%s, %s)\t", key, val);
  int i;

  if (is_value_ro(key, val)) return -1;

  if ((i = find_key(key)) > -1) {
    LOG_PRINTF(RED_ON"%s:%s -> %s\n"RED_OFF, key, key_value_pairs[i+1], val);
    memcpy(key_value_pairs[i+1], val, 32); // FIXME why 32?
    return 0;
  }
  LOG_PRINTF( RED_ON"%s: Unknown key\n"RED_OFF, key);
  return -1; // payton DUT returns 1 if nvram full, -1 if error, 0 if good
}

static int __set_printf(const char *key, const char *fmt, va_list args)
{
  LOG_PRINTF("__set_printf\t");
  int len;
  char buf[4096];
  len = vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if (!nvram_set(key, buf)) return len;
  return -1; // bad key
}

int nvram_set_printf(const char *key, const char *fmt, ...) {
  LOG_PRINTF("set_printf(%s,%s)\t", key, fmt);
  va_list args;
  va_start(args, fmt);
  return __set_printf(key, fmt, args);
  va_end(args);
}
int nvram_safe_set_printf(const char *key, const char *fmt, ...) {
  LOG_PRINTF("safe_set_printf\t");
  va_list args;
  va_start(args, fmt);
  return __set_printf(key, fmt, args);
  va_end(args);
}

/*
 m    m mmmmm   mmmm    mmm
 ##  ##   #    #"   " m"   "
 # ## #   #    "#mmm  #
 # "" #   #        "# #
 #    # mm#mm  "mmm#"  "mmm"   #

*/



inline static int find_key(const char *key) {
  int i;
  for (i = 0; i < kv_count; i+=2) if (!strcmp(key,key_value_pairs[i])) return i;
  return -1;
}

static int is_value_ro(const char *key, char *v) { // Dirty hack to prevent DUT overwriting sh*t
  static char* ro[] = {"http_client_ip" ,"http_from", "access_flag", "login_time", 0};
  for (char **ptr = ro; *ptr && **ptr; ptr++) {
    if (!strcmp(*ptr, key)) {
      LOG_PRINTF( RED_ON"Asked to modify RO value: %s -> %s\n"RED_OFF,key, v);
      return 1;
    }
  }
  return 0;
}

int nvram_invmatch(const char *key, const char *value) {
  LOG_PRINTF("invmatch\t");
  return (!nvram_match(key, value)); // TODO: check if invmatch works like this
}

int nvram_match(const char *key, const char *value) {
  LOG_PRINTF("match(%s,%s)\t", key, value);
  int i;

  if ((i = find_key(key)) > -1) {
    LOG_PRINTF("%s == %s\n",key,key_value_pairs[i+1]);
    return (!strcmp(key_value_pairs[i+1], value));
  } else {
    LOG_PRINTF( RED_ON"%s == Unknown\n"RED_OFF,key);
    return 0;
  }
}

void nvram_unset(const char *key) {
  LOG_PRINTF("unset(%s)\t", key);
  int i;
  if ((i = find_key(key)) > -1) {
    LOG_PRINTF(RED_ON"%s UNSET!!\n"RED_OFF,key);
    key_value_pairs[i+1] = 0;
  }
}

int nvram_commit(int a) {
  LOG_PRINTF("nvram_commit(%d)\t", a);
  return 1;
}

