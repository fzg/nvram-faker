#include <sys/types.h>
#include "nvram-faker.h"

/*
char *nvram_get(const char *key);
char *nvram_get_ex(const char *key, char **val, size_t len);

int nvram_match(const char *a, const char *b);
void nvram_set(const char *key, char *value);
void nvram_unset(const char *key);

*/
int main() {

// get
  char *mdl = nvram_get("model_name");
  if (!mdl) puts("string is null"); else printf("%s\n", mdl);
// match
  if (nvram_match("model_name", "prout")) puts("oops");
// overwrite
  nvram_set("model_name", "prout");
  if (nvram_match("model_name", "prout")) puts("prout:)");

// reget
  mdl = nvram_get("router_name");
  printf("%s\n", mdl);
// unset
  nvram_unset("router_name");
  mdl = nvram_get("router_name");
  if (!mdl) puts("string is null-> good");
  nvram_set("model_name", "proutophage");
  mdl = nvram_get("model_name");
  if (!mdl) puts("string is null"); else printf("%s\n", mdl);

  puts("get_scanf");

  nvram_get_scanf("model_name", "%s", &mdl);
  if (!mdl) puts("string is null"); else printf("%s\n", mdl);




// get after

  _exit(0);

}
