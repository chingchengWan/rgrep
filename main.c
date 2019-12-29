#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#define MAX_LINE_BYTE   4096
#define MAX_RECORD_BYTE 8192
#define MAX_READ_BUFFER 8192


void remove_backslash0(char *recBuff, off_t size)
{
    off_t i;
    for (i = 0; i < size; i++)
    {
        if(recBuff[i] == '\0')
        {
            recBuff[i] = ' ';
        }
    }
    return;
}


void get_record(FILE *fd, char *recBuff, int mode, char *recBgKey)
{
    int lenOfrecBgKey;
    off_t retOfRead;
    char *p2begin;


    //
    lenOfrecBgKey = strlen(recBgKey);
    recBuff[0] = '\0';
    // line-mode -> read a line seperated by '\n'.
    if(mode == 0)
    {
        fgets(recBuff, MAX_LINE_BYTE, fd);
        if(recBuff[strlen(recBuff) - 1] != '\n')
        {
            recBuff[strlen(recBuff)] = '\n';
            recBuff[strlen(recBuff) + 1] = '\0';
        }
    }

    // record-mode -> read a record according @recBgKey
    else if(mode == 1)
    {
        retOfRead = fread(recBuff, 1, MAX_RECORD_BYTE, fd);
        recBuff[retOfRead] = '\0';
        if(strlen(recBuff) != retOfRead)
        {
            remove_backslash0(recBuff, retOfRead);
            assert(strlen(recBuff) == retOfRead);
        }
        p2begin = strstr(recBuff, recBgKey);
        if(p2begin == NULL)
        {
            recBuff[0] = '\0';
            return;
        }

        // when p2begin starts from &recBuff[0]
        if(p2begin != recBuff)
        {
            fseeko(fd, -(retOfRead - (p2begin - recBuff)), SEEK_CUR);
            retOfRead = fread(recBuff, 1, MAX_RECORD_BYTE, fd);
            recBuff[retOfRead] = '\0';
            if(strlen(recBuff) != retOfRead)
            {
                remove_backslash0(recBuff, retOfRead);
                assert(strstr(recBuff, recBgKey) == recBuff);
            }
        }
        p2begin = strstr(recBuff + lenOfrecBgKey, recBgKey);
        if (p2begin == NULL)
        {
            recBuff[retOfRead] = '\0';
        }
        else
        {
            fseeko(fd, -(retOfRead - (p2begin - recBuff)), SEEK_CUR);
            (*p2begin) = '\0';
        }   
    }

    return;
}


void pat_search(char *recBuff, char *keyPatt, int mode, char *fieldQry)
{
    if(mode == 1 || (mode == 0 && fieldQry == NULL))
    {
        if(strstr(recBuff, keyPatt) != NULL)
        {
            printf("%s", recBuff);
            //exit(1);
        }
        return;
    }
    else if(mode == 0 && fieldQry != NULL)
    {
        if(strstr(recBuff, fieldQry) == recBuff &&  strstr(recBuff, keyPatt) != NULL)
        {
            printf("%s", recBuff);
            //exit(1);
        }
        return;
    }
}


void file_search(char *fpath, char *keyPatt, char *recBgKey, char *fieldQry)
{
    // open the file
    FILE *fd;
    fd = fopen( fpath, "rb");
    assert(fd != NULL);
    
    
    // decide the mode
    // 0 : line-mode
    // 1 : record-mode
    int mode = 0;
    mode = (recBgKey == NULL || strcmp(recBgKey, "\n") == 0) ? 0 : 1;
    /*
    if(recBgKey == NULL || strcmp(recBgKey, "\n") == 0)
    {
        mode = 0;
    }
    else
    {
        mode = 1;
    }
    */
    assert(fieldQry == NULL || mode == 0);
    


    // malloc size for record
    char *recBuff;
    recBuff = malloc(sizeof(char) * MAX_READ_BUFFER + 1);
    assert(recBuff != NULL);


    
    // main loop
    while( !feof(fd) )
    {
        get_record(fd, recBuff, mode, recBgKey);
        pat_search(recBuff, keyPatt, mode, fieldQry);
    }


    // fclose
    fclose(fd);

    // free memory
    free(recBuff);

    return;    
}


int main(int argc, char *argv[])
{
    char fpath[]    = "./ettoday.rec";
    char keyPatt[]  = "萬萬";
    char recBgKey[] = "\n";
    char fieldQry[] = "@title:";

    file_search(fpath, keyPatt, recBgKey, fieldQry);
    return 0;
}