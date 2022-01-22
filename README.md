# PIPI : PERFECT PARSER

PIPI is a two-headers library written in plain C to parse Json streams and files.

<br>

# INFO

PIPI treats all json elements as pointers to **JEntity**.

```
typedef struct JEntity
{
    int type;
    void *val;

}JEntity;
```

According to **type**, the pointer **val** will be :

**JOBJECT** --> pointer to allocated struct JObject

**JARRAY** --> pointer to allocated struct JArray

**JSTRING** --> pointer to allocated string

**JNUMBER** --> pointer to allocated string

**JKEYWORD** --> pointer to static string (**JNULL**,**JTRUE** or **JFALSE**)

Like you can see PIPI treats all the primitive elements (string,number and keyword) as simple strings.

Is up to the caller to make the correct conversion if required.

<br>

# FUNCTIONS

Some PIPI functions can set an error string in case of failure, you can retrieve it using **JSonGetError()**

System errors are handled with assertions.

<br>

## char *JSonGetError()

Retrieves the error string of the first PIPI function that failed.
Returns NULL if no errors occurred.

<br>

## void JSonClearError()

Clear the error string if set, otherwise do nothing

<br>

## JEntity *JSonParseStream(char *stream, int stream_size, int plat, int tab)

Parse a json stream returning a pointer to the root JEntity

**stream** : the stream to parse

**stream_size** : the length of the stream

**plat** : the type of EOL used, must one of the following : **UNIX**, **MAC** or **WIN**

**tab** : how may spaces to add to the stream position when encountering the special character TAB

*Can set error*

<br>

## JEntity *JSonParseFile(char *file, int plat, int tab)

Parse an entire json file returning a pointer to the root JEntity

**file** : the name of the file

**plat** : the type of EOL used, must one of the following : **UNIX**, **MAC** or **WIN**

**tab** : how may spaces to add to the stream position when encountering the special character TAB

*Can set error*

<br>

## void JSonClose(JEntity *entity)

Recursively free all the allocated entities

**entity** : pointer to the root JEntity

<br>

## JEntity *JSonGetName(JEntity *entity, char *name)

Given a root JEntity, returns a pointer to the JEntity with the corresponding name or NULL if not found

**entity** : pointer to JEntity ( must be of type **JOBJECT** )

**name** : the name of the entity to search for

*Can set error*

<br>

## JEntity *JSonGetIndex(JEntity *entity, int index)

Given a root JEntity, returns a pointer to the JEntity located at the matching position or NULL if index out of range

**entity** : pointer to JEntity ( must be of type **JARRAY** )

**index** :  position to retrieve

*Can set error*

<br>

## void JSonPrint(JEntity *entity, char *file)

Given a root JEntity, recursively print on text file all the nested entities

**entity** : pointer to JEntity

**file** : name of the file, if **NULL** print to stdout

*Can set error*

<br>

## JEntity *JSonMake(int type, void *val)
Return a pointer to JEntity, according to **type** parameter val parameter must be :

**JOBJECT** --> **NULL**

**JARRAY** --> **NULL**

**JSTRING** --> a valid string pointer

**JNUMBER** --> a valid string pointer

**JKEYWORD** --> **JNULL**,**JTRUE** or **JFALSE**

*Can set error*

<br>

## void JSonAdd(JEntity *entity, JEntity *ent)
Append any JEntity to a root JEntity of type JARRAY

**entity** pointer to root JEntity of type **JARRAY**

**ent** pointer to any JEntity to be added

*Can set error*

<br>

## void JSonSet(JEntity *entity, char *str, JEntity *ent)
Assign to a root JEntity of type JOBJECT a name and an entity

**entity** pointer to a root JEntity of type **JOBJECT**

**str** name of the object

**ent** pointer to any JEntity to be set

*Can set error*

<br>

## void JSonMerge(JEntity *entity, JEntity *ent)
Merge two JEntity of type object, appending the parameter JEntity to the root JEntity

**entity** pointer to root JEntity of type **JOBJECT**

**ent** pointer to JEntity of type **JOBJECT** to be added

*Can set error*

<br>

# TESTS

The tests are created using the suite JSON_checker available at : https://www.json.org/JSON_checker/

fail01.json is excluded because RFC 8259 allow a Json composed of single string. 

fail18.json is excluded because in RFC 8259 the depth of a Json is not specified.

Memory leaks have been checked with the folowing Valgrind flags :

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s

<br>

# TODO

- Change the parsing of strings from multiple realloc to a single calloc