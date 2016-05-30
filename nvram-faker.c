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

static int ini_handler(void *user, const char *section, const char *name,const char *value)
{

    int old_kv_len;
    char **kv;
    char **new_kv;
    int i;
    
    if(NULL == user || NULL == section || NULL == name || NULL == value)
    {
        DEBUG_PRINTF("bad parameter to ini_handler\n");
        return 0;
    }
    kv = *((char ***)user);
    if(NULL == kv)
    {
        LOG_PRINTF("kv is NULL\n");
        return 0;
    }
    
    DEBUG_PRINTF("kv_count: %d, key_value_pair_len: %d\n", kv_count,key_value_pair_len);
    if(kv_count >= key_value_pair_len)
    {
        old_kv_len=key_value_pair_len;
        key_value_pair_len=(key_value_pair_len * 2);
        new_kv=(char **)malloc(key_value_pair_len * sizeof(char **));
        if(NULL == kv)
        {
            LOG_PRINTF("Failed to reallocate key value array.\n");
            return 0;
        }
        for(i=0;i<old_kv_len;i++)
        {
            new_kv[i]=kv[i];
        }
        free(*(char ***)user);
        kv=new_kv;
        *(char ***)user=kv;
    }
    DEBUG_PRINTF("Got %s:%s\n",name,value);
    kv[kv_count++]=strdup(name);
    kv[kv_count++]=strdup(value);
    
    return 1;
}

void initialize_ini(void)
{
    int ret;
    DEBUG_PRINTF("Initializing.\n");
    if (NULL == key_value_pairs)
    {
        key_value_pairs=malloc(key_value_pair_len * sizeof(char **));
    }
    if(NULL == key_value_pairs)
    {
        LOG_PRINTF("Failed to allocate memory for key value array. Terminating.\n");
        exit(1);
    }
    
    ret = ini_parse(INI_FILE_PATH,ini_handler,(void *)&key_value_pairs);
    if (0 != ret)
    {
        LOG_PRINTF("ret from ini_parse was: %d\n",ret);
        LOG_PRINTF("INI parse failed. Terminating\n");
        free(key_value_pairs);
        key_value_pairs=NULL;
        exit(1);
    }else
    {
        DEBUG_PRINTF("ret from ini_parse was: %d\n",ret);
    }
    
    return;
    
}

void end(void)
{
    int i;
    for (i=0;i<kv_count;i++)
    {
        free(key_value_pairs[i]);
    }
    free(key_value_pairs);
    key_value_pairs=NULL;
    
    return;
}

char *nvram_get_ex(const char *key, char **val, size_t len)
{
 char *ptr = nvram_get(key);
// memset(*val, 0, len);
// memcpy(*val, ptr, len);
 *val = ptr;
 return &ptr;
}


// aweful memory leak but don't care for now
char *nvram_get(const char *key)
{
    int i;
    int found=0;
    char *value;
    char *ret;
    for(i=0;i<kv_count;i+=2)
    {
        if(strcmp(key,key_value_pairs[i]) == 0)
        {
            LOG_PRINTF("%s=%s\n",key,key_value_pairs[i+1]);
            found = (key_value_pairs[i+1]);
            value=key_value_pairs[i+1];
            break;
        }
    }

    ret = NULL;
    if(!found)
    {
            LOG_PRINTF( RED_ON"%s=Unknown\n"RED_OFF,key);
    }else
    {

            ret=strdup(value);
    }
    return ret;
}


int nvram_match(const char *key, const char *my_value) {
    int i, found = 0;
    char *value;
    for(i=0; i < kv_count; i+=2) {
        if(!strcmp(key,key_value_pairs[i])) {
            LOG_PRINTF("%s=%s\n",key,key_value_pairs[i+1]);
            found = 1;
            value=key_value_pairs[i+1];
            break;
        }
    }
    if(!found) {
       LOG_PRINTF( RED_ON"%s=Unknown\n"RED_OFF,key);
       return 0;
    }
    else {
      return (!strcmp(value, my_value));
    }

}

char is_value_ro(const char *key, char *v) {
    static char* ro[] = {"http_client_ip" ,"http_from", 0};

    for (char **ptr = ro; *ptr && **ptr; ptr++) {
     if (!strcmp(*ptr, key)) {
         LOG_PRINTF( RED_ON"Asked to modify RO value: %s -> %s\n"RED_OFF,key, v);
         return 1;
      }
    }
}

void nvram_set(const char *key, char *new_value) {
    int i, found = 0;
    char *value;
    if (is_value_ro(key, new_value)) return;
    for(i=0; i < kv_count; i+=2) {
        if(!strcmp(key,key_value_pairs[i])) {
            LOG_PRINTF(RED_ON"%s:%s -> %s\n"RED_OFF,key,key_value_pairs[i+1], new_value);
            found = 1;
            value=key_value_pairs[i+1];
            break;
        }
    }
    if(!found) LOG_PRINTF( RED_ON"%s=Unknown\n"RED_OFF,key);
    else {
     memcpy(value, new_value, 32);
    }
}

void nvram_unset(const char *key) {
    int i, found = 0;
    char *value;
    for(i=0; i < kv_count; i+=2) {
        if(!strcmp(key,key_value_pairs[i])) {
            LOG_PRINTF(RED_ON"%s UNSET!!\n"RED_OFF,key);
            found = 1;
            value=key_value_pairs[i+1];
	    key_value_pairs[i+1] = 0;

            break;
        }
    }
    if(!found) LOG_PRINTF( RED_ON"%s=Unknown\n"RED_OFF,key);
    else value = 0;
}


char *nvram_get_scanf(const char *key, const char *fmt, void **dest) {
  char *it = nvram_get(key);
  char *ptr = malloc(strlen(it));
  if (ptr) {
     sscanf(ptr, fmt, it);
     dest = ptr;
  }
}

