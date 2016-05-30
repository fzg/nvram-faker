#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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


static int ini_handler(void *user, const char *section, const char *name,const char *value) {
    int i, old_kv_len;
    char **kv, **new_kv;

    if (!user || !section || !name || !value) DIE("bad parameter to ini_handler\n");
    kv = *((char ***)user);
    if (!kv) DIE("kv is NULL\n");
    DEBUG_PRINTF("kv_count: %d, key_value_pair_len: %d\n", kv_count, key_value_pair_len);
    if (kv_count >= key_value_pair_len) {
        old_kv_len = key_value_pair_len;
        key_value_pair_len = (2*key_value_pair_len);
        new_kv = (char **)malloc(key_value_pair_len * sizeof(kv));
        if (!kv) DIE("Failed to reallocate key value array.\n");
        for (i = 0; i < old_kv_len; ++i) new_kv[i] = kv[i];
        free(*(char ***)user);
        kv = new_kv;
        *(char ***)user = kv;
    }
    DEBUG_PRINTF("Got %s:%s\n",name,value);
    kv[kv_count++] = strdup(name);
    kv[kv_count++] = strdup(value);
    return 1;
}

void initialize_ini(void) {
    int err;
    DEBUG_PRINTF("Initializing.\n");
    if (!key_value_pairs)
      if ((key_value_pairs = malloc(key_value_pair_len * sizeof(key_value_pairs))) == NULL)
        SIE("Failed to allocate memory for key value array. Terminating.\n");

    if ((err = ini_parse(INI_FILE_PATH, ini_handler, (void *)&key_value_pairs)) < 0) {
        LOG_PRINTF("ret from ini_parse was %d\nINI parse failed. Terminating\n", err);
        exit(1);
    }
}

void end(void) {
    for (int i = 0; i < kv_count; ++i) free(key_value_pairs[i]);
    free(key_value_pairs);
    key_value_pairs = NULL;
}

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

char *nvram_get_scanf(const char *key, const char *fmt, void **dest) {
  LOG_PRINTF("get_scanf ( %s %s )\t", key, fmt);
  static char *ptr = NULL;
  char *it = nvram_get(key);
  if (!it) {
    LOG_PRINTF("non existing %s!\n", key);
    it = "0";
  }
  if (ptr) { free(ptr); ptr = NULL; }
  ptr = malloc(strlen(it));
  if (ptr) {
     sscanf(ptr, fmt, it);
     dest = (void**)&ptr;
  }
  return ptr;
}

// TODO fix because apparently libnvram doesnt strdup for simple get (but for get_ex_2)
// aweful memory leak but don't care for now
char *nvram_get(const char *key) {
  LOG_PRINTF("get\t");
  int i, found = 0;
  char *value, *ret = NULL;
  if ((i = find_key(key)) > -1) {
    LOG_PRINTF("%s=%s\n",key,key_value_pairs[i+1]);
    found = !(!(key_value_pairs[i+1]));
    return found? strdup(key_value_pairs[i+1]) : NULL;
  }
  LOG_PRINTF( RED_ON"%s=Unknown\n"RED_OFF,key);
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
  LOG_PRINTF("safe_set\t");
  if (!val || (val && !key)) return -1;
  else return nvram_set(key, val);
}

int nvram_set(const char *key, char *new_value) {
  LOG_PRINTF("set\t");
  int i;

  if (is_value_ro(key, new_value)) return -1;

  if ((i = find_key(key)) > -1) {
    LOG_PRINTF(RED_ON"%s:%s -> %s\n"RED_OFF,key,key_value_pairs[i+1], new_value);
    memcpy(key_value_pairs[i+1], new_value, 32); // FIXME why 32?
    return 0;
  }
  LOG_PRINTF( RED_ON"%s=Unknown\n"RED_OFF,key);
  return -1; // payton DUT returns 1 if nvram full, -1 if error, 0 if good
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

int nvram_invmatch(const char *key, const char *my_value) {
  LOG_PRINTF("invmatch\t");
  return (!nvram_match(key, my_value)); // TODO: check if invmatch works like this
}

int nvram_match(const char *key, const char *my_value) {
  LOG_PRINTF("match\t");
  int i;

  if ((i = find_key(key)) > -1) {
    LOG_PRINTF("%s=%s\n",key,key_value_pairs[i+1]);
    return (!strcmp(key_value_pairs[i+1], my_value));
  } else {
    LOG_PRINTF( RED_ON"%s=Unknown\n"RED_OFF,key);
    return 0;
  }
}

void nvram_unset(const char *key) {
  LOG_PRINTF("unset\t");
  int i;
  if ((i = find_key(key)) > -1) {
    LOG_PRINTF(RED_ON"%s UNSET!!\n"RED_OFF,key);
    key_value_pairs[i+1] = 0;
  }
}

