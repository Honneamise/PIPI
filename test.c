//std 
#include "dirent.h"
#include "stdio.h"
#include "string.h"

//json
#include "JSON/json.h"

//main
int main()  
{
	DIR *dir = opendir("TEST");

	if(dir)
	{
		struct dirent *dp = NULL;

		while((dp = readdir(dir)) != NULL) 
		{
			if(dp->d_type==DT_REG)
			{
				char file[512] = {0};

				sprintf(file,"TEST/%s",dp->d_name);

				//excluded fail1.json and fail18.json due to RFC 8259
				if(strstr(file,"EXCLUDED")!=NULL){ continue; }

				JEntity *ent = JSonParseFile(file,UNIX,4);

				if(ent)
				{
					if(strncmp("pass",dp->d_name,4)!=0){ printf("-->"); }
					printf("%s : PASS\n",dp->d_name);
					JSonClose(ent);
				}
				else
				{
					if(strncmp("fail",dp->d_name,4)!=0){ printf("-->"); }
					printf("%s : FAIL\n",dp->d_name);
					JSonClearError();
				}

				
			}
    	}
        
		closedir(dir);

	}

	printf("If the JSON_checker is working correctly, it must accept all of the pass*.json files and reject all of the fail*.json files.\n");

	return 0;
	
}