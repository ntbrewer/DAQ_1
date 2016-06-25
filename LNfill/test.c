#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include "lnfill.h"

int main() 
{
    char lnfill_conf[200]="include/lnfill.conf";   //see define statement in lnfill.h 
    char line[200];
    FILE *ifile;
  //        char resolved_path[100]; 

	//        realpath("include/lnfill.conf", resolved_path); 
  //        printf("\n%s\n",resolved_path);
  char buff[PATH_MAX+1];
        printf("1:\n%s\n",getcwd(buff,PATH_MAX+1)); 

  if ( ( ifile = fopen (lnfill_conf,"r+") ) == NULL)
    {
      printf ("*** File on disk (%s) could not be opened: \n",lnfill_conf);
      printf ("===> %s \n",lnfill_conf);
      exit (EXIT_FAILURE);
    }

  fgets(line,150,ifile);    // reads column headers
  printf("%s\n",line);
        printf("\n%s\n",realpath("./include/lnfill.conf",NULL)); 

  printf("%s\n",line);
        printf("2:\n%s\n",getcwd(buff,PATH_MAX+1)); 
      int mm = fclose(ifile);


	return 0; 
}
