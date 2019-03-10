/*
 * Author: alcubierre-drive
 * Original source: http://github.com/alcubierre-drive/libsimpleconf
 * License: GNU AFFERO GENERAL PUBLIC LICENSE -- Version 3
 *          https://www.gnu.org/licenses/agpl-3.0.txt
 */

#ifndef LIBSIMPLECONF_H
#define LIBSIMPLECONF_H

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif
// the four different types that are supported by this library
#define CHAR_TYPE 1
typedef struct {
    char val[512];
    bool has;
} scfg_obj_char;
typedef struct {
    int val;
    bool has;
} scfg_obj_int;
#define INT_TYPE 2
typedef struct {
    float val;
    bool has;
} scfg_obj_float;
#define FLOAT_TYPE 3
typedef struct {
    bool val;
    bool has;
} scfg_obj_bool;
#define BOOL_TYPE 4

// a default struct containing all possible values a field can have. The user
// needs to initialize a lookuptable containing one defaults struct for each
// config key.
typedef struct {
    char c[512];
    int i;
    bool b;
    float f;
} defaults;

// each field in the simpleconf struct has a key that is searched for in the
// configuration fiel, a numerical key that is used throughout the program and a
// type (one of the four defined above). Note that also the defaults are
// contained within this struct.
typedef struct {
    char *key;
    int val;
    int type;
    defaults def;
} keystruct;

// all configuration options are saved within this struct. it contains a pointer
// to the lookuptable (table of keystructs) and the actual data
typedef struct {
    void** values;
    int number;
    keystruct* lookuptable;
} simpleconf;

// initialize the configuration. needs a simpleconf* with number and lookuptable
// already set.
void scfg_init( simpleconf* );

// possibility to modify the configuration with a user-defined function that
// takes the numeric config key and the corresponding configuration field
void scfg_modify( simpleconf*, void (*)( int key, void* cf_obj ) );

// function to read configuration from file and overwrite default values
void scfg_read_file( simpleconf*, char* path );

// get values from the configuration struct and use them in the main program
void scfg_read_val( const simpleconf*, int key, void* out);

// delete all allocated memory
void scfg_delete( simpleconf* );

#ifdef DEBUG
void scfg_dbg( const simpleconf* );
#endif

#ifdef __cplusplus
};
#endif

// sample implementation
// ---------------------

/*
 * #define HEIGHT 0
 * #define WIDTH 1
 * #define AREA 2
 * #define VOLUME 3
 * #define NAME 4
 * #define ADDRESS 5
 *
 * const defaults deflts[] = {
 * {"", 0, false, 1.0 },
 * {"", 0, false, 2.0 },
 * {"", 0, false, 2.0 },
 * {"", 0, false, 4.0 },
 * {"rectangle", 0, false, 0.0 },
 * {"right here", 0, false, 0.0 }
 * };
 *
 * keystruct lookuptable[] = {
 * { "height", HEIGHT, FLOAT_TYPE, deflts[0] },
 * { "width", WIDTH, FLOAT_TYPE, deflts[1] },
 * { "area", AREA, FLOAT_TYPE, deflts[2] },
 * { "volume", VOLUME, FLOAT_TYPE, deflts[3] },
 * { "name", NAME, CHAR_TYPE, deflts[4] },
 * { "address", ADDRESS, CHAR_TYPE, deflts[5] }
 * };
 *
 * simpleconf cfg;
 * cfg.lookuptable = lookuptable;
 * cfg.number = 6;
 * scfg_init( &cfg );
 * scfg_read_file( &cfg, "/etc/house.cfg" );
 *
 * float area;
 * scfg_read_val( &cfg, AREA, (void*)&area );
 * printf("The configured area is %.3f\n", area);
 *
 * scfg_delete( &cfg );
 */


// file '/etc/house.cfg'
// ---------------------

/* # this line is ignored
 * // so is this one
 *
 * area=1.0
 * width=0.5
 * height=0.7
 * name=my-own-house
 */
#endif // LIBSIMPLECONF_H
