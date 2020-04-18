/* Jorge Avila 1001543128
 Edrik Aguilera 1001729678
 CSE 3320 - Operating Systems
 
 Run: (1) gcc mfs.c -o mfs
      (2) ./mfs
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
//prototypes
void Open_File(char *filename);
void print_info();
void decimalToHex(int decimal_number);
void ls();
int cd(int index);
void stat(int index);
void get(char *filename, int index);
void read_from_file(char *filename, int position, int num_bytes, int index);
int LBAToOffset(int32_t sector);
int compare(char *filename, int *index);

#define WHITESPACE " \t\n" // We want to split our command line up into tokens
// so we need to define what delimits our tokens.
// In this case  white space
// will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5 // Mav shell only supports five arguments

//global variables
FILE *fp;

int16_t BPB_BytesPerSec;
int8_t BPB_SecPerClus;
int16_t BPB_RsvdSecCnt;
int8_t BPB_NumFATS;
int32_t BPB_FATSz32; //size

int root_cluster;
int in_root;

typedef struct __attribute__((__packed__)) DirectoryEntry
{
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t Unused1[8];
    uint16_t DIR_FirstClusterHigh;
    uint8_t Unused2[4];
    uint16_t DIR_FirstClusterLow;
    uint32_t DIR_FileSize;

} DirectoryEntry;

DirectoryEntry Dir[16];

int main(int argc, char **argv)
{

    char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);

    while (1)
    {
        // Print out the mfs prompt
        printf("mfs> ");

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
            ;

        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;

        // Pointer to point to the token
        // parsed by strsep
        char *arg_ptr;

        char *working_str = strdup(cmd_str);

        // we are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;

        // Tokenize the input stringswith whitespace used as the delimiter
        while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
               (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
            {
                token[token_count] = NULL;
            }
            token_count++;
        }

        // When user hits enter with no command, just continue to run the Mav Shell
        // with no other output.
        if (token[0] == NULL)
        {
            continue;
        }

        // Attempt to open the disk image file
        else if (strcmp(token[0], "open") == 0)
        {

            // If the file is already open do not open it again
            if (fp != NULL)
            {
                printf("A file is already opened.\n");
                continue;
            }

            // Otherwise go ahead and open the file
            // by calling Open_File() with a valid filename
            if (token[1] != NULL)
            {
                Open_File(token[1]);
            }
            else if (token[1] == NULL)
            {
                fprintf(stderr, "Error: You must provide an disk image to open\n");
            }
        }

        // Check to see if there is a file open to close
        // if so close it otherwise display an error
        else if (strcmp(token[0], "close") == 0)
        {
            if (fp != NULL)
            {
                fclose(fp);
                fp = NULL;
            }
            else
            {
                fprintf(stderr, "Error: No file open to close.\n");
            }
        }

        // Display information about the file system
        // by calling print_info() which will print 5 values
        // in both hexadecimal and base 10
        else if (fp != NULL && strcmp(token[0], "info") == 0)
        {
            print_info();
        }

        // Display attributes of a given file/directory
        // by calling stat(), display errors if the user
        // does not provide a file or provides a file that does not exist
        else if (fp != NULL && strcmp(token[0], "stat") == 0)
        {
            if (token[1] == NULL)
            {
                fprintf(stderr, "Error: Needs file / directory.\n");
            }
            else
            {
                int found;
                int index = -1;

                found = compare(token[1], &index);

                if (found)
                {
                    stat(index);
                }
                else
                {
                    fprintf(stderr, "Error: %s not found.\n", token[1]);
                }
            }
        }

        // Change the directory within the disk image by calling cd()
        // display errors if no directory provided, if the file provided is not a directory
        // and if no directory with the given name was found
        else if (fp != NULL && strcmp(token[0], "cd") == 0)
        {
            if (token[1] == NULL)
            {
                fprintf(stderr, "Error: Needs file directory.\n");
            }
            else if (strcmp(token[1], ".") == 0)
            {
                // Do nothing
                continue;
            }
            else
            {
                int found;
                int index = -1;

                found = compare(token[1], &index);

                if (found)
                {
                    int valid = cd(index);

                    if (!valid)
                    {
                        fprintf(stderr, "Error: %s not a directory.\n", token[1]);
                    }
                }
                else
                {
                    if ( in_root && strcmp(token[1], "..") == 0 )
                        continue;
                    else
                        fprintf(stderr, "Error: %s not found.\n", token[1]);
                }
            }
        }

        // List the contents of the directory within the disk image
        // by calling ls()
        else if (fp != NULL && strcmp(token[0], "ls") == 0)
        {
            ls();
        }

        // Copy the file from the disk image onto the current working directory
        // using the get() function, display an error if no file given, or if the file
        // is not found
        else if (fp != NULL && strcmp(token[0], "get") == 0)
        {
            if (token[1] == NULL)
            {
                fprintf(stderr, "Error: Needs file directory.\n");
            }
            else
            {
                int found;
                int index = -1;

                found = compare(token[1], &index);

                if (found)
                {
                    get(token[1], index);
                }
                else
                {
                    fprintf(stderr, "Error: %s not found.\n", token[1]);
                }
            }
            
        }

        else if (fp != NULL && strcmp(token[0], "read") == 0)
        {
            if (token[1] == NULL || token[2] == NULL || token[3] == NULL)
            {
                fprintf(stderr, "ERROR: Usage: read <filename> <position> <number of bytes>.\n");
            }
            else
            {
                int found;
                int index = -1;

                found = compare(token[1], &index);

                if (found)
                {
                    int position = atoi(token[2]);
                    int num_bytes = atoi(token[3]);
                    read_from_file(token[1], position, num_bytes, index);
                }
                else
                {
                    fprintf(stderr, "Error: %s not found.\n", token[1]);
                }
            }
            
        }

        // Gives the user two methods of quitting the Mav File System
        // User can enter quit or exit prompting the loop to break and
        // return EXIT_SUCCESS
        else if (strcmp(token[0], "exit") == 0 || (strcmp(token[0], "quit")) == 0)
        {
            // Close the file if it is still open
            // when user types exit
            if (fp != NULL)
            {
                fclose(fp);
            }

            printf("Exiting...\n");
            break;
        }

        // Display a different error depending on if
        // the Disk image is open or not
        else
        {
            if (fp != NULL)
            {
                fprintf(stderr, "Error: Command not found.\n");
            }
            else
            {
                fprintf(stderr, "Error: File system image must be opened first.\n");
            }
        }

        free(working_root);
    }

    return EXIT_SUCCESS;
}

int compare(char *filename, int *index)
{
    // Create a copy of the filename
    // so as not to mess with it in main
    char temp[12];
    strcpy(temp, filename);

    // Create a new string that is padded with spaces
    // to compare with the contents of the disk image
    char expanded_name[12];
    memset(expanded_name, ' ', 12);

    // If it is ".." then do not attempt to parse
    // the filename
    if (strncmp(filename, "..", 2) == 0)
    {
        strncpy(expanded_name, temp, 2);
    }

    // Otherwise begin parsing the file name
    // and formatting it correctly
    else
    {
        // Parse the first half of the file 
        char *tokenPtr = strtok(temp, ".");
        strncpy(expanded_name, tokenPtr, strlen(tokenPtr));
        // Then extract the extension
        tokenPtr = strtok(NULL, ".");

        // If there is an extension then add it to expanded_name
        if (tokenPtr)
        {
            strncpy((char *)(expanded_name + 8), tokenPtr, strlen(tokenPtr));
        }
        // NULL terminate the string
        expanded_name[11] = '\0';

        // Then change the case of the letters to be all uppercase
        // as FAT32 stores the filenames in all caps
        int k;
        for (k = 0; k < 11; k++)
        {
            expanded_name[k] = toupper(expanded_name[k]);
        }
    }

    // After properly formatting the string, begin the comparison
    int i;
    for (i = 0; i < 16; ++i)
    {
        // Once you find the file in the DirectoryEntry array
        // update the index of where the file is found
        // and return true
        if (strncmp(expanded_name, Dir[i].DIR_Name, 11) == 0)
        {
            *index = i;
            return 1;
        }
    }

    // If it is not found return false
    return 0;
}

void Open_File(char *filename)
{
    // If you cannot open the image, display an error
    // and return back to the main loop.
    if ((fp = fopen(filename, "r")) == NULL)
    {
        fprintf(stderr, "Error: Unable to open image %s.\n", filename);
        return;
    }

    // Read in the BIOS Parameter Block information
    fseek(fp, 11, SEEK_SET);
    fread(&BPB_BytesPerSec, 2, 1, fp);

    fseek(fp, 13, SEEK_SET);
    fread(&BPB_SecPerClus, 2, 1, fp);

    fseek(fp, 14, SEEK_SET);
    fread(&BPB_RsvdSecCnt, 2, 1, fp);

    fseek(fp, 16, SEEK_SET);
    fread(&BPB_NumFATS, 2, 1, fp);

    fseek(fp, 36, SEEK_SET);
    fread(&BPB_FATSz32, 4, 1, fp);

    // Set the root cluster
    root_cluster = (BPB_NumFATS * BPB_FATSz32 * BPB_BytesPerSec) +
                   (BPB_RsvdSecCnt * BPB_BytesPerSec);

    // Set the current directory entry to be the root directory
    fseek(fp, root_cluster, SEEK_SET);
    fread(&Dir[0], 16, sizeof(DirectoryEntry), fp);
    in_root = 1;
}

//this functions prints directory contents
void ls()
{
    int i;
    if (!in_root)
        printf(".\n");

    // Display all valid files
    for (i = 0; i < 16; ++i)
    {
        if ((Dir[i].DIR_Attr == 0x01 || Dir[i].DIR_Attr == 0x10 ||
             Dir[i].DIR_Attr == 0x20))
        {
            // Null terminate the file name
            // to produce nicer output
            char name[12];
            memset(name, 0, 12);
            strncpy(name, Dir[i].DIR_Name, 11);

            // Extract the first character to test
            // if the file has been deleted
            // deleted files have the first char as 0xe5
            // and must not be printed
            uint8_t first_char = name[0];
            if (first_char != 0xe5)
                printf("%s\n", name);
        }
    }
}

// Print the BIOS Parameter Block information
void print_info()
{
    printf("BPB_BytesPerSec : %d\nBPB_BytesPerSec : %x\n\n", BPB_BytesPerSec, BPB_BytesPerSec);
    printf("BPB_SecPerClus : %d\nBPB_SecPerClus : %x\n\n", BPB_SecPerClus, BPB_SecPerClus);
    printf("BPB_RsvdSecCnt : %d\nBPB_RsvdSecCnt : %x\n\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
    printf("BPB_NumFATS : %d\nBPB_NumFATS : %x\n\n", BPB_NumFATS, BPB_NumFATS);
    printf("BPB_FATSz32 : %d\nBPB_FATSz32 : %x\n\n", BPB_FATSz32, BPB_FATSz32);
}

// Display the attributes of the given file
void stat(int index)
{
    printf("%-18s%-8s%-23s", "Attribute", "Size", "Starting Cluster Number\n");
    printf("%-18d%-8d%-23d\n", Dir[index].DIR_Attr, Dir[index].DIR_FileSize,
           Dir[index].DIR_FirstClusterLow);
}

// Copy the content of the file onto the current working directory
void get(char *filename, int index)
{
    FILE *newFile;

    // Attempt to create the empty file
    // if it fails display an error
    if ((newFile = fopen(filename, "w")) == NULL)
    {
        fprintf(stderr, "Error: Unable to create file %s.\n", filename);
        return;
    }

    // Allocate enough room to hold the data of the file
    uint32_t size = Dir[index].DIR_FileSize;
    void *content = malloc(size);

    // Store the content into the void pointer
    // which will be used to generate the file
    // on the local working directory
    fseek(fp, LBAToOffset(Dir[index].DIR_FirstClusterLow), SEEK_SET);
    fread(content, size, 1, fp);

    // Populate the file then close it once finished
    fwrite(content, size, 1, newFile);
    fclose(newFile);
}

void read_from_file(char *filename, int position, int num_bytes, int index)
{
    if (Dir[index].DIR_FileSize == 0)
    {
        fprintf(stderr, "ERROR: Cannot read from directory.\n");
    }
    else
    {
        char *content = malloc(num_bytes);
        int offset = LBAToOffset(Dir[index].DIR_FirstClusterLow);
        fseek(fp, offset + position, SEEK_SET);
        fread(content, num_bytes, 1, fp);
        int i;
        for (i = 0; i < num_bytes; ++i)
        {
            uint8_t curr_char = (uint8_t) content[i];
            printf("%x ", curr_char);
        }
        printf("\n");
    }
    
}

// Change the directory within the fat32 disk image
int cd(int index)
{
    // Check to see if it is a directory
    // only directories have size 0
    if (Dir[index].DIR_FileSize == 0)
    {
        // If it is the root cluster then
        // FirstClusterLow will be 0 and you can 
        // fseek to the root, and update in_root to be true
        if (Dir[index].DIR_FirstClusterLow == 0)
        {
            in_root = 1;
            fseek(fp, root_cluster, SEEK_SET);
            fread(&Dir[0], 16, sizeof(DirectoryEntry), fp);
            return 1;
        }
        
        // For all other directories calculate the offset
        // then fseek to that cluster location and update
        // in_root to be false
        in_root = 0;
        int offset = LBAToOffset(Dir[index].DIR_FirstClusterLow);
        fseek(fp, offset, SEEK_SET);
        fread(&Dir[0], 16, sizeof(DirectoryEntry), fp);
        return 1;
    }
    else
    {
        // If its not a directory present an error
        // in main
        return 0;
    }
}

int LBAToOffset(int32_t sector)
{
    return ((sector - 2) * BPB_BytesPerSec + root_cluster);
}

