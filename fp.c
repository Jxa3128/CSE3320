/*
  Name & ID: Jennifer Ulloa 1001263031
  Name & ID: Jorge Avila 1001543128
  How to run: gcc fp.c and then do ./a.out sample.txt
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "page_algorithm.h"

#define MAX_LINE 1024

int main( int argc, char * argv[] )
{
  char * line = NULL;
  size_t line_length = MAX_LINE;
  char * filename;

  FILE * file;

  if( argc < 2 )
  {
    printf("Error: You must provide a datafile as an argument.\n");
    printf("Example: ./fp datafile.txt\n");
    exit( EXIT_FAILURE );
  }

  filename = ( char * ) malloc( strlen( argv[1] ) + 1 );
  line     = ( char * ) malloc( MAX_LINE );

  memset( filename, 0, strlen( argv[1] + 1 ) );
  strncpy( filename, argv[1], strlen( argv[1] ) );

  printf("Opening file %s\n", filename );
  file = fopen( filename , "r");
  int i=0;
  int numberArray [10000];
  if ( file )
  {
    while ( fgets( line, line_length, file ) )
    {
      char * token;

      token = strtok( line, " ");
      int working_set_size = atoi( token );

      printf("\nWorking set size: %d\n", working_set_size );

      while( token != NULL )
      {
        token = strtok( NULL, " " );
        if( token != NULL )
        {
           //printf("Request: %d\n", atoi( token ) );
           numberArray[i] = atoi(token);
           //printf("Array: %d\n", numberArray[i]);
           i++;
        }
      }
      printf("\n");
      int fifo_page_faults = FIFO(numberArray,i, working_set_size);
      int ULR_page_faults=MRU(numberArray,i,working_set_size);
      int LRU_page_faults = LRU(numberArray,i, working_set_size);
      int optimal_page_faults=Optimal(numberArray,i,working_set_size);
      /* These are the outputs for the 4 algorithms */
      printf("page faults of FIFO:\t%d\n",fifo_page_faults);
      printf("page faults of LRU:\t%d\n",LRU_page_faults);
      printf("page faults of MRU:\t%d\n",ULR_page_faults);
      printf("page faults of Optimal:\t%d\n",optimal_page_faults);
      //printf("THE FIRST ELEMENT OF STRING == %d\n",working_set_size);
      memset(numberArray, 0, 255);
      i=0;
    }

    free( line );
    fclose(file);
  }

  return 0;
}
