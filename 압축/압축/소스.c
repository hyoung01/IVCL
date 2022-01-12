#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define width 352
#define height 288
#define block_size 16
#define prediction_number 128
#define quantization_number 10
#define number_frame 2

int block_wcount = width / block_size;
int block_hcount = height / block_size; 
int resoulution_size = width * height * (3 / 2.0);
int bb = 0;
int block[block_size][block_size];
int Bitstream[number_frame][width * height] = { 0 };
unsigned char org[number_frame][width*432] = { 0 };


void partition(int f, int b_c, int block[][block_size])
{
    int i, j, k;
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            k = ((b_c) / block_wcount) * block_size * width + ((b_c) % block_wcount) * block_size + i * block_wcount * block_size + j;
            block[i][j] = org[f][k]; // block배열에 값추가
        }
    }
}

void write(int f, int b_c, int block[][block_size], unsigned char** recon)
{
    int i, j, k;
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++) // 압축한거 다시 삽입
        {
            k = ((b_c) / block_wcount) * block_size * width + ((b_c) % block_wcount) * block_size + i * block_wcount * block_size + j;
            recon[f][k] = block[i][j];
        }
    }
}

void read_bitstream(int f, int block[][block_size], int Bitstream[][width * height])
{
    int i, j;
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            block[i][j] = Bitstream[f][bb];
            bb++;
        }
    }
}

void write_bitstream(int f, int block[][block_size], int Bitstream[][width * height])
{
    int i, j;
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            Bitstream[f][bb] = block[i][j];
            bb++;
        }
    }
}

void Prediction(int block[][block_size])
{
    int i, j;
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            block[i][j] = block[i][j] - prediction_number;
        }
    }
}

void Quantization(int block[][block_size])
{
    int i, j;
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            block[i][j] = (int)(block[i][j] / quantization_number);
        }
    }
}

void Inverse_Quantization(int block[][block_size])
{
    int i, j, k;
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            k = block[i][j] * quantization_number;
            if (k > 255) block[i][j] = 255;
            else block[i][j] = k;
        }
    }
}

void Recon(int block[][block_size])
{
    int i, j, k;
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            k = block[i][j] + prediction_number;
            if (k < 0) block[i][j] = 0;
            else if (k > 255) block[i][j] = 255;
            else block[i][j] = k;
            //printf("%d ", block[i][j]);
        }
    }
}

double PSNR(unsigned char** recon)
{

    double mse = 0, psnr=0;
    for (int f = 0; f < number_frame; f++)
    {
        mse = 0;
        for (int i = 0; i < width * height; i++)
        {
            mse += (org[f][i] - recon[f][i]) * (org[f][i] - recon[f][i]);
        }
        mse = mse / (width * height);
        psnr += 10 * log10((255 * 255) / mse);
    }
    return (psnr/number_frame);

}

void encoding(FILE* pOrg, FILE* pRecon_e, FILE* pBits_e)
{
    int i, j, b_c = 0;

    unsigned char** recon_e;


    recon_e = (unsigned char**)malloc(sizeof(unsigned char*) * number_frame);

    for (i = 0;i < number_frame; i++)
    {
        recon_e[i] = (unsigned char*)malloc(sizeof(unsigned char) * resoulution_size);
    }

    for (i = 0;i < number_frame; i++)   
    {
        fread(org[i], 1, resoulution_size, pOrg);
    }

    for (i = 0;i < number_frame; i++) // 회색
    {
        for (j = width * height;j < resoulution_size; j++)
        {
            recon_e[i][j] = 128;
        }
    }

    for (int f = 0; f < number_frame; f++) // 프레임
    {
        bb = 0;
      
        for (b_c = 0; b_c < block_hcount * block_wcount; b_c++) // 16*16 블록
        {
            partition(f, b_c, block);
            Prediction(block);
            Quantization(block);
            Inverse_Quantization(block);
            Recon(block);
            write(f, b_c, block, recon_e);
            write_bitstream(f, block, Bitstream);
        }
        
        fwrite(recon_e[f], 1, resoulution_size, pRecon_e);
        fwrite(Bitstream[f], 1, width * height, pBits_e);

    }
     printf("%.3f\n", PSNR(recon_e));
}

void decoding(FILE* pRecon_d )
{
    int i, j, b_c = 0;
    FILE* pBits_d = NULL;
    pBits_d = fopen("./bitstream.txt", "rb");

    unsigned char** recon_d;
    recon_d = (unsigned char**)malloc(sizeof(unsigned char*) * number_frame);

    for (i = 0;i < number_frame; i++)
    {
        recon_d[i] = (unsigned char*)malloc(sizeof(unsigned char) * resoulution_size);
    }


    for (i = 0;i < number_frame; i++)
    {
        fread(Bitstream[i], 1, width * height, pBits_d);
    }

    for (i = 0;i < number_frame; i++) // 회색
    {
        for (j = width * height;j < resoulution_size; j++)
        {
            recon_d[i][j] = 128;
        }
    }

    for (int f = 0; f < number_frame; f++) // 프레임
    {
        bb = 0;

        for (b_c =0; b_c < block_hcount * block_wcount; b_c++) // 16*16 블록
        {
            
            read_bitstream(f, block, Bitstream);
            Prediction(block);
            Quantization(block);
            Inverse_Quantization(block);
            Recon(block);
            write(f, b_c, block, recon_d);
        }
        fwrite(recon_d[f], 1, resoulution_size, pRecon_d);
    }
    printf("%.3f\n", PSNR(recon_d));


}


int main()
{
    int i, c = 1, b_c = 0, b_c_h = 0, k = 0;
    double psnr = 0;
    FILE* pOrg = NULL;
    FILE* pRecon_e = NULL;
    FILE* pRecon_d = NULL;
    FILE* pBits_e = NULL;

    pOrg = fopen("C:\\Users\\최혜영\\Desktop\\FOREMAN_CIF30.yuv", "rb");
    pRecon_e = fopen("./newe_y_352x288.yuv", "wb");
    pRecon_d = fopen("./newd_y_352x288.yuv", "wb");
    pBits_e = fopen("./bitstream.txt", "wb");

    if (pOrg == NULL || pRecon_e == NULL || pRecon_d == NULL)
    {
        printf("File error");
        return -1;
    }   

    encoding(pOrg, pRecon_e, pBits_e);
    decoding(pRecon_d);

    fclose(pRecon_e);
    fclose(pRecon_d);
    fclose(pBits_e);
    fclose(pOrg);
    return 0;
}