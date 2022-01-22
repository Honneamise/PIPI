/***********/
/* HEADERS */
/***********/
//std
#include "assert.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

//stuff
#define IS
#include "is.h"

static char *JERR = NULL;

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

#ifdef JSON
/*********************/
/* STATIC PROTOTYPES */
/*********************/
static void JSonSetError(char *str, Is *is);

static int JSonDigitCharacter(char c);
static int JSonHexDigitCharacter(char c);
static int JSonControlCharacter(char c);
static int JSonSpecialCharacterUnescaped(char c);

static void JSonStringAdd(char **s, char c);

static void JSonFileToStream(char *file, char **stream, int *stream_size);

static char *JSonParseNumber(Is *is);
static char *JSonParseString(Is *is);
static JArray *JSonParseArray(Is *is);
static JObject *JSonParseObject(Is *is);
static JEntity *JSonParseEntity(Is *is);

static void JSonCloseArray(JArray *array);
static void JSonCloseObject(JObject *object);
static void JSonCloseEntity(JEntity *entity);

static void JSonPrintTabs(FILE *fp, int n);
static void JSonPrintArray(JArray *array, FILE *fp, int lvl);
static void JSonPrintObject(JObject *object, FILE *fp, int lvl);
static void JSonPrintEntity(JEntity *entity, FILE *fp, int lvl);

/********************/
/* STATIC FUNCTIONS */
/********************/
static void JSonSetError(char *str, Is *is)
{
    if(!JERR)
    {
        JERR = calloc(128,sizeof(char));

        assert(JERR!=NULL);

        if(is)
        {
            sprintf(JERR,"%s (pos:%d line:%d col:%d)",str,is->pos,is->line,is->col);
        }
        else
        {
            sprintf(JERR,"%s",str); 
        }
    }
}

/**********/
static int JSonDigitCharacter(char c)
{
    //0..9 A..Z a..z 
    return (c>=48 && c<=57);
}

/**********/
static int JSonHexDigitCharacter(char c)
{
    //0..9 A..F a..f 
    return (c>=48 && c<=57) || (c>=65 && c<=70) || (c>=97 && c<=102);
}

/**********/
static int JSonControlCharacter(char c)
{
    return (c>=0x00 && c<=0x1F) || c== 0x7F;
}

/**********/
static int JSonSpecialCharacterUnescaped(char c)
{
    return  c=='"' || 
            c=='\\' ||
            c=='/'  ||
            c=='b' ||
            c=='f' ||
            c=='n' ||
            c=='r' ||
            c=='t' ;
}

/**********/
static void JSonStringAdd(char **s, char c)
{
    int len = strlen(*s)+2;

    char *ptr = realloc(*s,len*sizeof(char));

    assert(ptr!=NULL);

    ptr[len-2]=c;
    ptr[len-1]=0;

	*s = ptr;
}

/**********/
static void JSonFileToStream(char *file, char **stream, int *stream_size)
{
	size_t _size = 0;

	FILE *fp = fopen(file, "rb");

	assert(fp!=NULL);
    
    fseek(fp, 0, SEEK_END);

	_size = ftell(fp);

	*stream = calloc( _size+1, sizeof(char));//+1 for end of line

    assert(stream!=NULL);

	fseek(fp, 0, SEEK_SET);

	fread(*stream, _size, 1, fp);

	fclose(fp);

	if(stream_size!=NULL){ *stream_size = (int)_size; }
}

/**********/
static char *JSonParseNumber(Is *is)//check for IsEnd after IsNext
{
    char *s = calloc(1,sizeof(char));

    assert(s!=NULL);

    if(IsAccept(is,'-')) 
    { 
        JSonStringAdd(&s,'-'); 
    }

    //inter0
    if(IsAccept(is,'0'))
    {
        JSonStringAdd(&s,'0');
    }
    else if(IsPeek(is)>=49 && IsPeek(is)<=57)// 1..9
    {
        JSonStringAdd(&s,IsPeek(is));

        IsNext(is);

        while(!IsEnd(is) && JSonDigitCharacter(IsPeek(is)))
        {
            JSonStringAdd(&s,IsNext(is));
        }
    }
    else//error
    {
        JSonSetError("Expected digit",is); 
        free(s);
        return NULL;
    }

    //parte frazionale
    if(IsAccept(is,'.'))
    {
        JSonStringAdd(&s,'.');

        if(IsEnd(is) || !JSonDigitCharacter(IsPeek(is)))//error 
        { 
            JSonSetError("Expected digit",is); 
            free(s);
            return NULL;
        }

        while(!IsEnd(is) && JSonDigitCharacter(IsPeek(is)))
        {
            JSonStringAdd(&s,IsNext(is));
        }
    }

    //parte esponente
    if(IsPeek(is)=='E' || IsPeek(is)=='e')
    {
        JSonStringAdd(&s,IsNext(is));

        if(IsPeek(is)=='+' || IsPeek(is)=='-')
        {
            JSonStringAdd(&s,IsNext(is));
        }

        if(IsEnd(is) || !JSonDigitCharacter(IsPeek(is)))//error 
        { 
            JSonSetError("Expected digit",is); 
            free(s);
            return NULL;
        }

        while(!IsEnd(is) && JSonDigitCharacter(IsPeek(is)))
        {
            JSonStringAdd(&s,IsNext(is));
        }
    }

    return s;
}

/**********/
static char *JSonParseString(Is *is)
{
    if(!IsExpect(is,'"')) 
    { 
        JSonSetError("Expected (\")",is); 
        return NULL;
    }

    char *s = calloc(1,sizeof(char));

    assert(s!=NULL);

    int clean = 1;

    while(!IsAccept(is, '"'))
    {
        if(IsEnd(is)) { clean = 0; break; }

        if(IsPeek(is)=='\\')//escape found
        {
            JSonStringAdd(&s,IsNext(is));

            if(IsEnd(is)) { clean = 0; break; }

            if( JSonSpecialCharacterUnescaped(IsPeek(is)) )//special character unescaped
            {
                JSonStringAdd(&s,IsNext(is));

                if(IsEnd(is)) { clean = 0; break; }
            }
            else if(IsPeek(is)=='u')//hex digit x4
            {
                JSonStringAdd(&s,IsNext(is));

                if(IsEnd(is)) { clean = 0; break; }

                for(int i=0;i<4;i++)
                {
                    if(JSonHexDigitCharacter(IsPeek(is)))
                    {
                        JSonStringAdd(&s,IsNext(is));

                        if(IsEnd(is)) { clean = 0; break; }
                    }
                    else//error
                    {
                        JSonSetError("Invalid unicode sequence",is);
                        clean = 0; 
                        break;
                    }
                }
            }
            else//error
            {
                JSonSetError("Invalid escape character",is);
                clean = 0; 
                break;
            }

        }
        else
        {
            if(JSonControlCharacter(IsPeek(is)) )//error control character found
            {
                JSonSetError("Unexpected control character found",is);
                clean = 0; 
                break;
            }
            else
            {
                JSonStringAdd(&s,IsNext(is));

                if(IsEnd(is)) { clean = 0; break; }
            }
        }
        
    }

    if(!clean) 
    { 
        JSonSetError("Unexpected end of string",is);
        free(s); 
        s = NULL; 
    }
   
    return s;
}

/**********/
static JArray *JSonParseArray(Is *is)
{
    if(!IsExpect(is,'[')) 
    { 
        JSonSetError("Expected ([)" ,is); 
        return NULL;
    }
  
    IsSkipBlanks(is);

    if(IsAccept(is,']'))//an empty array
    {
        JArray *array = calloc(1,sizeof(JArray));

        assert(array!=NULL);
        
        return array;
    }

    JArray *array = NULL;
    do
    {
        if(IsEnd(is))
        { 
            JSonSetError("End of stream missing array entity",is); 

            JSonCloseArray(array);

            return NULL;
        }

        JEntity *entity = JSonParseEntity(is);

        if(!entity)
        {
            JSonCloseArray(array);

            return NULL;
        }

        JArray *arr = calloc(1,sizeof(JArray));

        assert(arr!=NULL);

        arr->entity = entity;

        if(array==NULL){ array = arr; }
        else if(array->next==NULL)
        {
            array->next = arr;
            array->last = arr;
        }
        else
        {
            array->last->next = arr;
            array->last = arr;
        }

        array->size++;
        
    } while(IsAccept(is,','));

    if(!IsExpect(is,']')) 
    { 
        JSonSetError("Expected (])",is); 

        JSonCloseArray(array);

        return NULL;
    }

    return array;
}

/**********/
static JObject *JSonParseObject(Is *is)
{
    JObject *object = NULL;

    if(!IsExpect(is,'{')) 
    { 
        JSonSetError("Expected ({)" ,is); 
        return NULL;
    }

    IsSkipBlanks(is);

    if(IsAccept(is,'}'))//an empty object
    {
        JObject *object = calloc(1,sizeof(JObject));

        assert(object!=NULL);

        return object;
    }

    do
    {
        IsSkipBlanks(is);

        if(IsEnd(is))
        { 
            JSonSetError("End of stream missing object entity",is); 

            JSonCloseObject(object);

            return NULL;
        }

        char *name = JSonParseString(is);

        if(!name)
        {
            JSonCloseObject(object);
            return NULL;
        }

        IsSkipBlanks(is);

        if(!IsExpect(is,':')) 
        { 
            JSonSetError("Expected (:)" ,is); 
            
            free(name);

            JSonCloseObject(object);

            return NULL;
        }

        JEntity *entity = JSonParseEntity(is);

        if(!entity)
        {
            free(name);

            JSonCloseObject(object);

            return NULL;
        }

        JObject *obj = calloc(1,sizeof(JObject));
        assert(obj!=NULL);
        obj->name = name;
        obj->entity = entity;

        if(object==NULL){ object = obj; }
        else if(object->next==NULL)
        {
            object->next = obj;
            object->last = obj;
        }
        else
        {
            object->last->next = obj;
            object->last = obj;
        }
        
    } while(IsAccept(is,','));

    if(!IsExpect(is,'}')) 
    { 
        JSonSetError("Expected (})",is); 
            
        JSonCloseObject(object);

        return NULL;
    }

    return object;
}

/**********/
static JEntity *JSonParseEntity(Is *is)
{
    JEntity *entity = calloc(1,sizeof(JEntity));
    assert(entity!=NULL);

    IsSkipBlanks(is);

    if(IsPeek(is)=='{')
    {
        entity->type = JOBJECT;
        entity->val = JSonParseObject(is);
    }
    else if (IsPeek(is)=='[')
    {
        entity->type = JARRAY;
        entity->val = JSonParseArray(is);
    }
    else if (IsPeek(is)=='"')
    {
        entity->type = JSTRING;
        entity->val = JSonParseString(is);
    }
    else if (IsPeek(is)=='-' || JSonDigitCharacter(IsPeek(is)))
    {
        entity->type = JNUMBER;
        entity->val = JSonParseNumber(is);
    }
    else if(strncmp(&is->input[is->pos],JNULL, 4)==0)//NULL
    {
        entity->type = JKEYWORD;
        entity->val = JNULL;
        is->pos+=4;
    }
    else if(strncmp(&is->input[is->pos],"true", 4)==0)//TRUE
    {
        entity->type = JKEYWORD;
        entity->val = JTRUE;
        is->pos+=4;
    }
    else if(strncmp(&is->input[is->pos],"false", 5)==0)//FALSE
    {
        entity->type = JKEYWORD;
        entity->val = JFALSE;
        is->pos+=5;
    }
    else
    {
        //error
        JSonSetError("Unexpected symbol found",is);
        free(entity);
        entity = NULL;
    }

    IsSkipBlanks(is);

    if(entity && !entity->val)
    {
        free(entity); 
        entity = NULL;
    }

    return entity;
}

/**********/
static void JSonCloseArray(JArray *array)
{
    if(!array) { return; }

    JArray *arr = array;

    while(arr!=NULL)
    {
        void *tmp = arr->next;
        
        if(arr->entity) { JSonCloseEntity(arr->entity); }

        free(arr);

        arr = tmp;
    }
}

/**********/
static void JSonCloseObject(JObject *object)
{
    if(!object) { return; }

    JObject *obj = object;

    while(obj!=NULL)
    {
        void *tmp = obj->next;

        if(obj->name){ free(obj->name); }

        if(obj->entity) { JSonCloseEntity(obj->entity); }

        free(obj);
        
        obj = tmp;
    }
}

/**********/
static void JSonCloseEntity(JEntity *entity)
{	
    if(!entity) { return; }

    switch(entity->type)
    {
        case JOBJECT:
            if(entity->val) { JSonCloseObject(entity->val); }
            break;
    
        case JARRAY:
            if(entity->val) { JSonCloseArray(entity->val); }
            break;

        case JSTRING:
            if(entity->val) { free(entity->val); }
            break;

        case JNUMBER:
            if(entity->val) { free(entity->val); }
            break;

        case JKEYWORD:
            //do nothng,static string
            break;

        default:
            //err
            JSonSetError("Invalid entity type found",NULL);
            break;
    }

    free(entity);
}

/**********/
static void JSonPrintTabs(FILE *fp, int n)
{
    for(int i=0;i<n;i++) 
    {
        fprintf(fp,"    ");
    }
}

/**********/
static void JSonPrintArray(JArray *array, FILE *fp, int lvl)
{
    JArray *arr=array;
    
    fprintf(fp,"[\n");

    while(arr!=NULL)
    {        
        JSonPrintTabs(fp,lvl);
        JSonPrintEntity(arr->entity, fp, lvl+1);

        if(arr->next!=NULL) fprintf(fp,",\n");
        else fprintf(fp,"\n"); 

        arr=arr->next;
    }

    JSonPrintTabs(fp,lvl-1);
    fprintf(fp,"]");
}

/**********/
static void JSonPrintObject(JObject *object, FILE *fp, int lvl)
{
    JObject *obj=object;
    
    fprintf(fp,"{\n");

    while(obj!=NULL)
    {   
        if(obj->name && obj->entity)//if both NULL empty object, if only one null  is malformed
        {
            JSonPrintTabs(fp,lvl);  
            fprintf(fp,"\"%s\" : ",(char*)obj->name);
            
            JSonPrintEntity(obj->entity, fp, lvl+1);

            if(obj->next!=NULL) fprintf(fp,",\n");
            else fprintf(fp,"\n"); 
        }

        obj=obj->next;
    }

    JSonPrintTabs(fp,lvl-1);
    fprintf(fp,"}");
}

/**********/
static void JSonPrintEntity(JEntity *entity, FILE *fp, int lvl)
{
    switch(entity->type)
    {
        case JOBJECT:
            if(entity->val) { JSonPrintObject(entity->val, fp, lvl); }
            break;
    
        case JARRAY:
            if(entity->val) { JSonPrintArray(entity->val, fp, lvl); }
            break;

        case JSTRING:
            if(entity->val) { fprintf(fp,"\"%s\"",(char*)entity->val); }
            break;

        case JNUMBER:
            if(entity->val) { fprintf(fp,"%s",(char*)entity->val); }
            break;

        case JKEYWORD:
            if(entity->val) { fprintf(fp,"%s",(char*)entity->val); }
            break;

        default:
            //err
            JSonSetError("Invalid entity type found",NULL);
            break;
    }
}

/********************/
/* PUBLIC FUNCTIONS */
/********************/
char *JSonGetError()
{
    return JERR;
}

/**********/
void JSonClearError()
{
    if(JERR) { free(JERR); JERR = NULL;}
}

/**********/
JEntity *JSonParseStream(char *stream, int stream_size, int plat, int tab)
{
    if(!stream || stream_size<=0) 
    { 
        JSonSetError("Empty stream",NULL);
        return NULL; 
    }

    Is is = IsInit(stream, stream_size, plat, tab);

    JEntity *entity = JSonParseEntity(&is);

    if(entity && !IsEnd(&is)) 
    { 
        JSonSetError("Expected end of file",&is);
        JSonCloseEntity(entity);
        entity = NULL;
    }

    return entity;
}

/**********/
JEntity *JSonParseFile(char *file, int plat, int tab)
{
    if(!file) { JSonSetError("File name cannot be NULL",NULL); };

    char *stream = NULL;
	int stream_size = 0;

    JSonFileToStream(file,&stream, &stream_size);

    if(!stream) 
    { 
        JSonSetError("Failed to read file",NULL);
        return NULL;
    }

    JEntity *entity = JSonParseStream(stream, stream_size, plat, tab);

    free(stream);

    return entity;
}

/**********/
void JSonClose(JEntity *entity)
{
    JSonCloseEntity(entity);
}

/**********/
JEntity *JSonGetName(JEntity *entity, char *name)
{
    if(!entity)
    { 
        JSonSetError("Entity cannot be NULL",NULL);
        return NULL; 
    }
    
    if(entity->type!=JOBJECT)
    { 
        JSonSetError("Entity type must be JOBJECT",NULL);
        return NULL; 
    }

    if(!name) 
    { 
        JSonSetError("Name cannot be NULL",NULL);
        return NULL; 
    }

    JObject *obj = entity->val;

    while(obj!=NULL)
    {
        if(strcmp(obj->name,name)==0){ return obj->entity; }
        obj = obj->next;
    }

    return NULL;
}

/**********/
JEntity *JSonGetIndex(JEntity *entity, int index)
{
    if(!entity)
    {
        JSonSetError("Entity cannot be NULL",NULL);
        return NULL;
    }

    if(entity->type!=JARRAY)
    {
        JSonSetError("Entity must be JARRAY",NULL);
        return NULL;
    }

    if( index<0 || index>=((JArray*)entity->val)->size )
    { 
        JSonSetError("Index out of range",NULL);
        return NULL; 
    }

    JArray *arr = entity->val;

    for(int i=0; i<index; i++)
    {
        arr = arr->next;
    }

    return arr->entity;
}

/**********/
void JSonPrint(JEntity *entity, char *file)
{
    if(!entity)
    {
        JSonSetError("Entity cannot be NULL",NULL);
        return;
    }

    FILE *fp = stdout;
    
    if(file) 
    { 
        fp = fopen(file, "w");

        assert(fp!=NULL);
    }

    JSonPrintEntity(entity,fp,1);

    if(fp!=stdout) { fclose(fp); }
}

/**********/
JEntity *JSonMake(int type, void *val)
{
    JEntity *entity = calloc(1,sizeof(JEntity));
    assert(entity!=NULL);

    entity->type = type;

    switch(type)
    {
        case JOBJECT:
            if(val){ JSonSetError("val parameter must be NULL",NULL); free(entity); return NULL; }//error

            entity->val = calloc(1,sizeof(JObject));
            assert(entity->val!=NULL);
            break;

        case JARRAY:
            if(val){ JSonSetError("val parameter must be NULL",NULL); free(entity); return NULL; }//error

            entity->val = calloc(1,sizeof(JArray));
            assert(entity->val!=NULL);
            break;

        case JSTRING:
            {
                if(!val){ JSonSetError("val parameter cannot be NULL",NULL); free(entity); return NULL; }//error

                int len = strlen((char*)val);
                entity->val = calloc(len+1,sizeof(char));
                assert(entity->val!=NULL);
                strcpy(entity->val,val);
            }
            break;

        case JNUMBER:
            {
                if(!val){ JSonSetError("val parameter cannot be NULL",NULL); free(entity); return NULL; }//error

                int len = strlen((char*)val);
                entity->val = calloc(len+1,sizeof(char));
                assert(entity->val!=NULL);
                strcpy(entity->val,val);
            }
            break;

        case JKEYWORD:
            if(!val){ JSonSetError("val parameter cannot be NULL",NULL); free(entity); return NULL; }//error

            if(
                strcmp(val,JNULL)!=0 &&
                strcmp(val,JTRUE)!=0 &&
                strcmp(val,JFALSE)!=0
                )
            {
                if(!val){ JSonSetError("Invalid keyword",NULL); }//error invalid keyword
            }

            entity->val = val;
            break;

        default:
            //error fuck YEAH !!!
            JSonSetError("Invalid entity type",NULL);
            free(entity); return NULL;
            break;
    }

    return entity;
}

/**********/
void JSonAdd(JEntity *entity, JEntity *ent)
{
    if(!entity)
    {
        JSonSetError("Entity cannot be NULL",NULL);  
        return; 
    }

    if(entity->type!=JARRAY)
    {
        JSonSetError("Entity type must be JARRAY",NULL);
        return;
    }

    if(!ent) 
    {
        JSonSetError("ent parameter cannot be NULL",NULL);  
        return; 
    }

    if(entity->type==JARRAY)//array
    {
        JArray *array = entity->val;

        if(array->entity==NULL)
        { 
            array->entity = ent; 
        }
        else if(array->next==NULL)
        {
            JArray *arr = calloc(1,sizeof(JArray));
            assert(arr!=NULL);
            arr->entity = ent;

            array->next = arr;
            array->last = arr;
        }
        else
        {
            JArray *arr = calloc(1,sizeof(JArray));
            assert(arr!=NULL);
            arr->entity = ent;

            array->last->next = arr;
            array->last = arr;
        }

        array->size++;

        return;
    }

}

/**********/
void JSonSet(JEntity *entity, char *str, JEntity *ent)
{
    if(!entity)
    {
        JSonSetError("Entity cannot be NULL",NULL);  
        return; 
    }

    if(entity->type!=JOBJECT)
    {
        JSonSetError("Entity type must be JOBJECT",NULL);
        return;
    }
    
    if(!str)
    {
        JSonSetError("Entity name cannot be NULL",NULL);
        return;
    }

    if(!ent) 
    {
        JSonSetError("ent parameter cannot be NULL",NULL);  
        return; 
    }

    JObject *obj = entity->val;

    if(obj->name && obj->entity) { return; } //err not empty object

    int len = strlen(str);

    obj->name = calloc(len+1,sizeof(char));
    assert(obj->name!=NULL);
    strcpy(obj->name,str);

    obj->entity = ent;

}

/**********/
void JSonMerge(JEntity *entity, JEntity *ent)
{
   if(!entity)
    {
        JSonSetError("Entity cannot be NULL",NULL);  
        return; 
    }

    if(entity->type!=JOBJECT)
    {
        JSonSetError("Entity type must be JOBJECT",NULL);
        return;
    }
    
    if(!ent)
    {
        JSonSetError("ent parameter cannot be NULL",NULL);
        return;
    }

    if(ent->type!=JOBJECT) 
    {
        JSonSetError("ent type must be JOBJECT",NULL);  
        return; 
    }

    if(entity->type==JOBJECT && ent->type==JOBJECT)//object
    {
        JObject *_check = NULL;

        //checks
        _check = entity->val;
        if(!_check || !_check->name || !_check->entity) { return; }//entity empty ?

        _check = ent->val;
        if(!_check || !_check->name || !_check->entity) { return; }//ent empty ?
        //end checks

        JObject *object = entity->val;

        if(object->next==NULL)
        {
            JObject *obj = ent->val;

            object->next = obj;
            object->last = obj;
        }
        else
        {
            JObject *obj = ent->val;
            
            object->last->next = obj;
            object->last = obj->last;
        }

        free(ent);

        return;
    }
   
    //error : entity not an OBJECT
    JSonSetError("Entity type must be JOBJECT",NULL);
    return;
}


#endif

#endif
