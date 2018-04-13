#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

int LBAToOffset(int32_t sector);
int16_t NextLB(uint32_t sector);



struct __attribute__((__packed__)) DirectoryEntry{
    char DIR_Name[11];
    uint8_t Dir_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;
};

char  OEMNAME[8];
int   BytsPerSec;
short SecPerClus;
int   RsvdSecCnt;
short NumFATs;
int   RootEntCnt;
long  FATSz32;
long  RootClus;
char  VolLab[11];		  
long FATOffset = 0;
long TotalFATSize = 0;
long ClusterOffset = 0;

int main(void)
{
	char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
	FILE* fp = NULL;
	int open = 0;
	//long Root_Directory = 0;

	while(1) //Infinite loop to run shell
	{
		printf ("msh> "); // Print shell prompt

		while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) ); // Wait for user input

	 	/* Parse input */
     	char *token[MAX_NUM_ARGUMENTS];

    	int   token_count = 0;                                 
                                                           
    	// Pointer to point to the token
    	// parsed by strsep
	    char *arg_ptr;                                         
                                                           
    	char *working_str  = strdup( cmd_str );                

    	// we are going to move the working_str pointer so
    	// keep track of its original value so we can deallocate
    	// the correct amount at the end
    	char *working_root = working_str;

    	// Tokenize the input stringswith whitespace used as the delimiter
    	while (((arg_ptr = strsep(&working_str, WHITESPACE )) != NULL) && (token_count<MAX_NUM_ARGUMENTS))
    	{
      		token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      		if( strlen( token[token_count] ) == 0 )
      		{
        		token[token_count] = NULL;
      		}
        	token_count++;
    	}

    	if(open == 0 && strcmp(token[0],"open") != 0)
    	{
    		printf("Error: File system image must be opened first.\n");
    		continue;
    	}
    	if(strcmp(token[0],"open") == 0)
    	{
    		fp = fopen(token[1],"r");
    		if(open != 0)
    			printf("Error: File system image already open.\n");
    		else if(fp != NULL)
    		{
    			open = 1;
 				fseek(fp, 3, SEEK_SET); //(stream, bytes, operation)
				fread(&OEMNAME, 8, 1, fp); //(address, length of item, number of items, stream)

				fseek(fp, 11, SEEK_SET); //(stream, bytes, operation)
				fread(&BytsPerSec, 2, 1, fp); //(address, length of item, number of items, stream)
			  
				fseek(fp, 13, SEEK_SET); //(stream, bytes, operation)
				fread(&SecPerClus, 1, 1, fp); //(address, length of item, number of items, stream)
			  
				fseek(fp, 14, SEEK_SET); //(stream, bytes, operation)
				fread(&RsvdSecCnt, 2, 1, fp); //(address, length of item, number of items, stream)
			  
				fseek(fp, 16, SEEK_SET); //(stream, bytes, operation)
				fread(&NumFATs, 1, 1, fp); //(address, length of item, number of items, stream)
			  
				fseek(fp, 17, SEEK_SET); //(stream, bytes, operation)
				fread(&RootEntCnt, 2, 1, fp); //(address, length of item, number of items, stream)
			  
				fseek(fp, 36, SEEK_SET); //(stream, bytes, operation)
				fread(&FATSz32, 4, 1, fp); //(address, length of item, number of items, stream)
			  
				fseek(fp, 44, SEEK_SET); //(stream, bytes, operation)
				fread(&RootClus, 4, 1, fp); //(address, length of item, number of items, stream)
			  
				fseek(fp, 71, SEEK_SET); //(stream, bytes, operation)
				fread(&VolLab, 11, 1, fp); //(address, length of item, number of items, stream)

				FATOffset = (RsvdSecCnt*BytsPerSec);
				TotalFATSize = (NumFATs*FATSz32*BytsPerSec);
				ClusterOffset = (FATOffset+TotalFATSize);
    		}
    		else
    			printf("Error: File system image not found.\n");
    	}

    	if(strcmp(token[0],"info") == 0)
    	{
			  
			printf("\nOEMNAME:\t%s\n", &OEMNAME);
			printf("BytsPerSec:\t%d\n", BytsPerSec);
			printf("SecPerClus:\t%d\n", SecPerClus);
			printf("RsvdSecCnt:\t%d\n", RsvdSecCnt);
			printf("NumFATs:\t%d\n", NumFATs);
			printf("RootEntCnt:\t%d\n", RootEntCnt);
			printf("FATSz32:\t%d\n", FATSz32);
			printf("RootClus:\t%d\n", RootClus);
			printf("VolLab:\t%s\n\n", &VolLab);
			  
			printf("FATOffset:\t%d\n", FATOffset);
			printf("TotalFATSize:\t%d\n", TotalFATSize);
	    	printf("ClusterOffset:\t%d\n\n", ClusterOffset);

	    	/*printf("Root Directory Data\n-----------------------\n");
	    	fseek(fp,(NumFATs*FATSz32*BytsPerSec) + (RsvdSecCnt*BytsPerSec),SEEK_SET);
	    	fread(&Root_Directory,16,32,fp);
	    	Root_Directory = (NumFATs*FATSz32*BytsPerSec) + (RsvdSecCnt*BytsPerSec);
	    	printf("%x",Root_Directory);*/

    	}

    	if(strcmp(token[0],"close") == 0)
    	{
    		if(open == 0)
    			printf("Error: File system not open.\n");
    		else
    		{
    			open = 0;
    			fclose(fp);
    		}
    	}
	}
}

int16_t NextLB(uint32_t sector,FILE *fp)
{
	uint32_t FATAddress = (BytsPerSec*RsvdSecCnt) + (sector * 4);
	int16_t val;
	fseek(fp, FATAddress,SEEK_SET);
	fread(&val,2,1,fp);
	return val;
}

int LBAToOffset(int32_t sector)
{
	return ((sector - 2) * BytsPerSec) + (BytsPerSec * RsvdSecCnt) + (NumFATs * FATSz32 * BytsPerSec);
}
