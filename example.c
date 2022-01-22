//std 
#include "stdio.h"

//json
#define JSON
#include "JSON/json.h"

//main
int main()  
{
    JEntity *str1 = JSonMake(JSTRING,"hello1");
	JEntity *obj1 = JSonMake(JOBJECT,NULL);
	JSonSet(obj1,"string1",str1);

	JEntity *str2 = JSonMake(JSTRING,"hello2");
	JEntity *obj2 = JSonMake(JOBJECT,NULL);
	JSonSet(obj2,"string2",str2);

	JEntity *str3 = JSonMake(JSTRING,"hello3");
	JEntity *obj3 = JSonMake(JOBJECT,NULL);
	JSonSet(obj3,"string3",str3);

	JSonMerge(obj1,obj2);
	JSonMerge(obj1,obj3);

    JEntity *arr = JSonMake(JARRAY,NULL);

    JSonAdd(arr,JSonMake(JOBJECT,NULL));
    JSonAdd(arr,JSonMake(JSTRING,"hello4"));
    JSonAdd(arr,JSonMake(JKEYWORD,JFALSE));
    JSonAdd(arr,obj1);

	JSonPrint(arr,NULL);

    printf("\n");

	JSonClose(arr);

    return 0;
}