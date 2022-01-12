#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main()
{

    int i, j, c=1;
    FILE* pFile = NULL;
    FILE* pWriteFile = NULL;

    pFile = fopen("C:\\Users\\ÃÖÇý¿µ\\Desktop\\FOREMAN_CIF30.yuv", "rb");
    pWriteFile = fopen("./oneframe_y.yuv", "wb");


    if (pFile == NULL || pWriteFile ==NULL)
    {
        printf("File error");
        return -1;
    }
    
    int resoulution_size = 352 * 288 * (3/2.0);
    int number_frame = 300;

    unsigned char** read_data;
    read_data = (unsigned char**)malloc(sizeof(unsigned char*) * number_frame);

    for (i = 0;i < number_frame; i++)
    {
        read_data[i] = (unsigned char*)malloc(sizeof(unsigned char) * resoulution_size);
    }

    for (i = 0;i < number_frame; i++)
    {
        fread(read_data[i], 1, resoulution_size, pFile);
    }
 
    for (i = 0;i < number_frame; i++)
    {
        for (j = 352*288 ;j < resoulution_size; j++)
        {
            read_data[i][j] = 128;
        }
    }
    for (i = 0;i < number_frame; i++)
    {
        fwrite(read_data[i], 1, resoulution_size, pWriteFile);
    }

    
    /*
    int resoulution_size = 352 * 288 * (3 / 2.0) * 300;
    unsigned char* read_data;
    read_data = (unsigned char*)malloc(sizeof(unsigned char) * resoulution_size);
    fread(read_data, 1, resoulution_size, pFile);


    for (i = 1; i <= 300; i++)
    {
        for (j = 352 * 288 * (3 / 2.0) * i ; j < resoulution_size; j++)
        {
            if (j > 352 * 288 * i)
            {
                read_data[i] = 128;
            }

        }
    }

    */
    fwrite(read_data, 1, resoulution_size, pWriteFile);
 

    fclose(pWriteFile);
    fclose(pFile);

    //getchar();


    return 0;
}