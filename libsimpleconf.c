/*
 * Author: alcubierre-drive
 * Original source: http://github.com/alcubierre-drive/libsimpleconf
 * License: GNU AFFERO GENERAL PUBLIC LICENSE -- Version 3
 *          https://www.gnu.org/licenses/agpl-3.0.txt
 */

#include "libsimpleconf.h"

#define BADKEY -1

static int keyfromstring( const simpleconf* cfg, char* key, int* type ) {
    for (int i=0; i < cfg->number; ++i) {
        if (strcmp(cfg->lookuptable[i].key, key) == 0) {
            *type = cfg->lookuptable[i].type;
            return cfg->lookuptable[i].val;
        }
    }
    return BADKEY;
}

static char* remove_string(char* str, const char* sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

static void scfg_obj_char_set( scfg_obj_char* x, char* y ) {
    strcpy(x->val,y);
    x->has = true;
}
static void scfg_obj_int_set( scfg_obj_int* x, char* y ) {
    x->val = atoi(y);
    x->has = true;
}
static void scfg_obj_float_set( scfg_obj_float* x, char* y ) {
    x->val = atof(y);
    x->has = true;
}
static void scfg_obj_bool_set( scfg_obj_bool* x, char* y ) {
    if (strcmp(y,"true") == 0) {
        x->val = true;
    } else {
        x->val = false;
    }
    x->has = true;
}

static void get_name( const simpleconf* cfg, int which, char* out ) {
    for (int i=0; i<cfg->number; ++i) {
        if (cfg->lookuptable[i].val == which) {
            strcpy(out,cfg->lookuptable[i].key);
            break;
        }
    }
}

static void cf_set( simpleconf* cfg, int which, char* val, int type ) {
    if (val==NULL) {
        char name[512];
        get_name(cfg,which,name);
        fprintf(stderr,"[libsimpleconf] Empty config field '%s'\n",name);
        return;
    }
    switch(type) {
        case CHAR_TYPE:
            scfg_obj_char_set( ((scfg_obj_char*)(cfg->values)[which]), val );
            break;
        case INT_TYPE:
            scfg_obj_int_set( ((scfg_obj_int*)(cfg->values)[which]), val );
            break;
        case FLOAT_TYPE:
            scfg_obj_float_set( ((scfg_obj_float*)(cfg->values)[which]), val );
            break;
        case BOOL_TYPE:
            scfg_obj_bool_set( ((scfg_obj_bool*)(cfg->values)[which]), val );
            break;
    }
}

static int get_type( const simpleconf* cfg, int which) {
    int type = -1;
    for (int i=0; i<cfg->number; ++i) {
        if (cfg->lookuptable[i].val == which) {
            type = cfg->lookuptable[i].type;
        }
    }
    return type;
}

static void modify_config( simpleconf* cfg, char* line ) {
    if (line[strspn(line, " \t\v\r\n")] == '\0') {
        // ignore empty lines
        return;
    }
    if (line[0] == '#' || (line[0] == '/' && line[1] == '/')) {
        // ignore comments
        return;
    }
    char delimiter[16] = "=\n";
    char keyval[512];
    strcpy(keyval,line);
    char* key;
    char* val;
    key = strtok(keyval,delimiter);
    val = strtok(NULL,delimiter);
    int type = BADKEY;
    int key_num = keyfromstring(cfg, key, &type);
    if (key_num != BADKEY) {
        cf_set( cfg, key_num, val, type );
    } else {
        fprintf(stderr, "[libsimpleconf] unknown key '%s'.\n", key);
    }
}

static void set_defaults( simpleconf* cfg ) {
    for (int i=0; i<cfg->number; ++i) {
        int idx = cfg->lookuptable[i].val;
        defaults def = cfg->lookuptable[i].def;
        int type = cfg->lookuptable[i].type;
        switch(type) {
            case CHAR_TYPE:
                if ( ((scfg_obj_char*)(cfg->values)[idx])->has ) {
                    break;
                }
                strcpy(((scfg_obj_char*)(cfg->values)[idx])->val,def.c);
                break;
            case INT_TYPE:
                if ( ((scfg_obj_int*)(cfg->values)[idx])->has ) {
                    break;
                }
                ((scfg_obj_int*)(cfg->values)[idx])->val = def.i;
                break;
            case BOOL_TYPE:
                if ( ((scfg_obj_bool*)(cfg->values)[idx])->has ) {
                    break;
                }
                ((scfg_obj_bool*)(cfg->values)[idx])->val = def.b;
                break;
            case FLOAT_TYPE:
                if ( ((scfg_obj_float*)(cfg->values)[idx])->has ) {
                    break;
                }
                ((scfg_obj_float*)(cfg->values)[idx])->val = def.f;
                break;
        }
    }
}

void scfg_read_file( simpleconf* cfg, char* path ) {
    FILE* fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "[libsimpleconf] could not read '%s'.\n", path);
        return;
    }
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, fp)) != -1) {
        modify_config( cfg, line );
    }
    free(line);
    fclose(fp);
}

void scfg_init( simpleconf* cfg ) {
    int number = cfg->number;
    cfg->values = malloc(number*sizeof(void*));
    keystruct* lookuptable = cfg->lookuptable;
    for (int i=0; i<number; ++i) {
        int idx = lookuptable[i].val;
        int type = lookuptable[i].type;
        int size = 0;
        switch(type) {
            case CHAR_TYPE:
                size = sizeof(scfg_obj_char);
                (cfg->values)[idx] = malloc(size);
                ((scfg_obj_char*)(cfg->values)[idx])->has = false;
                break;
            case INT_TYPE:
                size = sizeof(scfg_obj_int);
                (cfg->values)[idx] = malloc(size);
                ((scfg_obj_int*)(cfg->values)[idx])->has = false;
                break;
            case BOOL_TYPE:
                size = sizeof(scfg_obj_bool);
                (cfg->values)[idx] = malloc(size);
                ((scfg_obj_bool*)(cfg->values)[idx])->has = false;
                break;
            case FLOAT_TYPE:
                size = sizeof(scfg_obj_float);
                (cfg->values)[idx] = malloc(size);
                ((scfg_obj_float*)(cfg->values)[idx])->has = false;
                break;
        }
    }
    set_defaults( cfg );
}

void scfg_delete( simpleconf* cfg ) {
    for (int i=0; i<cfg->number; ++i) {
        free(cfg->values[i]);
    }
    free(cfg->values);
}

void scfg_read_val( const simpleconf* cfg, int which, void* out ) {
    int type = get_type(cfg, which);
    switch(type) {
        case CHAR_TYPE:
            strcpy(out,((scfg_obj_char*)((cfg->values)[which]))->val);
            break;
        case INT_TYPE:
            *((int*)out) = ((scfg_obj_int*)((cfg->values)[which]))->val;
            break;
        case BOOL_TYPE:
            *((bool*)out) = ((scfg_obj_bool*)((cfg->values)[which]))->val;
            break;
        case FLOAT_TYPE:
            *((float*)out) = ((scfg_obj_float*)((cfg->values)[which]))->val;
            break;
    }
}

void scfg_modify( simpleconf* cfg, void (*mod)( int key, void* cf_obj ) ) {
    for (int i=0; i<cfg->number; ++i) {
        int idx = cfg->lookuptable[i].val;
        (*mod)(idx,cfg->values[i]);
    }
}

void scfg_dbg( const simpleconf* cfg) {
    fprintf(stderr,"[libsimpleconf] dbg\nidx type value\n");
    for (int i=0; i<cfg->number; ++i) {
        int type = get_type(cfg,i);
        fprintf(stderr,"%i %i ",i,type);
        switch(type) {
            case CHAR_TYPE:
                fprintf(stderr,"%s\n",( (scfg_obj_char*)(cfg->values[i]) )->val);
                break;
            case INT_TYPE:
                fprintf(stderr,"%i\n",( (scfg_obj_int*)(cfg->values[i]) )->val);
                break;
            case BOOL_TYPE:
                fprintf(stderr,"%i\n",( (scfg_obj_bool*)(cfg->values[i]) )->val);
                break;
            case FLOAT_TYPE:
                fprintf(stderr,"%.3f\n",( (scfg_obj_float*)(cfg->values[i]) )->val);
                break;
        }
    }
}

#undef BADKEY
