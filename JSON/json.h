#ifndef _JPARSER_H
#define _JPARSER_H

#include "is.h"

/***********/
/* DEFINES */
/***********/
#define JNULL   "null"
#define JTRUE   "true"
#define JFALSE  "false"

#define JOBJECT     0//allocated struct
#define JARRAY      1//allocated struct
#define JSTRING     2//allocated string
#define JNUMBER     3//allocated string
#define JKEYWORD    4//static string 

/************/
/* TYPEDEFS */
/************/
typedef struct JEntity
{
    int type;
    void *val;

}JEntity;

typedef struct JObject
{
    char  *name;
    JEntity *entity;
    struct JObject *next;
    struct JObject *last;

}JObject;

typedef struct JArray
{
    int size;
    JEntity *entity;
    struct JArray *next;
    struct JArray *last;

}JArray;

/**********/
char *JSonGetError();

void JSonClearError();

JEntity *JSonParseStream(char *stream, int stream_size, int plat, int tab);

JEntity *JSonParseFile(char *file, int plat, int tab);

void JSonClose(JEntity *entity);

JEntity *JSonGetName(JEntity *entity, char *name);

JEntity *JSonGetIndex(JEntity *entity, int index);

void JSonPrint(JEntity *entity, char *file);

JEntity *JSonMake(int type, void *val);

void JSonAdd(JEntity *entity, JEntity *ent);

void JSonSet(JEntity *entity, char *str, JEntity *ent);

void JSonMerge(JEntity *entity, JEntity *ent);

#endif
