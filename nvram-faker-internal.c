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

