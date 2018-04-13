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

struct __attribute__((__packed__)) DirectoryEntry{
    char DIR_Name[11];
    uint8_t DIR_Attr;
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

FILE* fp;

long FATOffset = 0;
long TotalFATSize = 0;
long ClusterOffset = 0;

int i = 0;
int root = 0;
int current = 0;

struct DirectoryEntry dir[16];

int LBAToOffset ( int32_t sector )
{
   return ( (sector - 2) * BytsPerSec) + (BytsPerSec * RsvdSecCnt) + (NumFATs * FATSz32 * BytsPerSec );
}

int16_t NextLB ( uint32_t sector )
{
   uint32_t FATAddress = ( BytsPerSec * RsvdSecCnt ) + ( sector * 4 );
   int16_t val;
   fseek ( fp, FATAddress, SEEK_SET );
   fread ( &val, 2, 1, fp );
   return val;
}

void readdir()
{
    for (i=0;i<16;i++)
    {
       fread(&dir[i], 1, 32, fp);
    }
}


int main(void)
{
	char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
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

				root = ClusterOffset;
       			current = root;
       			fseek(fp, root, SEEK_SET);
				readdir();
    		}
    		else
    			printf("Error: File system image not found.\n");
    	}

    	if(strcmp(token[0],"info") == 0)
    	{
    		printf("Attribute\tBase-10\tHex\n---------------\t-------\t----\n");
			printf("BPB_BytsPerSec:\t%d\t%x\n", BytsPerSec, BytsPerSec);
			printf("BPB_SecPerClus:\t%d\t%x\n", SecPerClus, SecPerClus);
			printf("BPB_RsvdSecCnt:\t%d\t%x\n", RsvdSecCnt, RsvdSecCnt);
			printf("BPB_NumFATs:\t%d\t%x\n", NumFATs, NumFATs);
			printf("BPB_FATSz32:\t%ld\t%lx\n\n", FATSz32, FATSz32);
    	}

    	if(strcmp(token[0],"ls") == 0)
    	{

    		for ( i = 0; i < 16 ; i++)
          	{
             	if ( ( dir[i].DIR_Attr == 0x01 ) || ( dir[i].DIR_Attr == 0x10 ) || ( dir[i].DIR_Attr == 0x20 ) )
             	{
                	printf ("%s\t%x\n", strndup(dir[i].DIR_Name,11), LBAToOffset(dir[i].DIR_FirstClusterLow));
             	}
          	}
    	}

    	if(strcmp(token[0],"cd") == 0)
    	{

    	}

    	if(strcmp(token[0],"volume") == 0)
    	{
    		if(VolLab==NULL)
    		{
    			printf("Error: volume name not found\n");
    		}else{
    			printf("Volume name: \"%11.11s\"\n", VolLab);
    		}
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