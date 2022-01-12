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

int quantization_number_decoding;

int block[block_size][block_size];

int block_H[block_size][block_size];
int block_V[block_size][block_size];

int block_PH[block_size];
int block_PV[block_size];

int Bitstream[number_frame][width * height + 396 +1] = { 0 }; // 0과 1 담아둘 부분 = 블록 개수만큼 추가, quatization 값 담아둘 하나도 추가
unsigned char org[number_frame][width * 432] = { 0 };


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

void partition_recon(int f, int b_c, int block[][block_size] , unsigned char** recon) // prediction이나 recon에서 필요한 앞 블록 정보 가져오기
{
    int i, j = 0, k;

    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            k = ((b_c) / block_wcount) * block_size * width + ((b_c) % block_wcount) * block_size + i * block_wcount * block_size + j;
            block[i][j] = recon[f][k]; // block배열에 값추가
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

int read_bitstream(int f, int block[][block_size], int Bitstream[][width * height + 396 +1])
{
    int i, j, k;
    quantization_number_decoding = Bitstream[f][0];

    if (Bitstream[f][bb] == 1)
    {
        bb++;
        k = 1;
    }
    else if (Bitstream[f][bb] == 0)
    {
        bb++;
        k = 0;
    }

    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            block[i][j] = Bitstream[f][bb];
            bb++;
        }
    }
    return k;
}

void write_bitstream(int f, int block[][block_size], int Bitstream[][width * height + 396 + 1], int t)
{
    int i, j;

    Bitstream[f][0] = quantization_number;

    Bitstream[f][bb] = t; // 0,1보내기
    bb++;

    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            Bitstream[f][bb] = block[i][j];
            bb++;
        }
    }

}

int Prediction(int block[][block_size], int b_c, int f, unsigned char** recon_e)
{
    int i, j;

    int block_P[block_size][block_size];

    int r_H = 0, r_V = 0;

    if (b_c % 22 == 0) // 왼쪽 128
    {
        for (i = 0; i < block_size; i++)
        {
            for (j = 0; j < block_size; j++)
            {
                block_H[i][j] = block[i][j] - prediction_number;
            }
        }
        for (i = 0; i < block_size; i++)
        {
            block_PH[i] = prediction_number;
        }
    }

    else // 옆에 블록으로 prediction
    {
        partition_recon(f, b_c - 1, block_P, recon_e);
        for (i = 0; i < block_size; i++)
        {
            for (j = 0; j < block_size; j++)
            {
                block_H[i][j] = block[i][j] - block_P[i][15];
            }
        }
        for (i = 0; i < block_size; i++)
        {
            block_PH[i] = block_P[i][15];
        }
    }


    if (b_c < 22) // 위쪽 128
    {
        for (i = 0; i < block_size; i++)
        {
            for (j = 0; j < block_size; j++)
            {
                block_V[i][j] = block[i][j] - prediction_number;
            }
        }
        for (i = 0; i < block_size; i++)
        {
            block_PV[i] = prediction_number;
        }
    }

    else // 위에 블록으로 prediction
    {
        partition_recon(f, b_c - 22, block_P, recon_e);

        for (i = 0; i < block_size; i++)
        {
            for (j = 0; j < block_size; j++)
            {
                block_V[j][i] = block[j][i] - block_P[15][j];
               
            }
        }
        for (i = 0; i < block_size; i++)
        {
            block_PV[i] = block_P[15][i];
        }
    }
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            r_H += block[i][j] - block_H[i][j];
            r_V += block[i][j] - block_V[i][j];
        }
    }
    if (r_H >= r_V) // 위쪽 블록
    {
        for (i = 0; i < block_size; i++)
        {
            for (j = 0; j < block_size; j++)
            {
                block[i][j] = block_V[i][j];
            }
        }
        return 1;
    }
    else if (r_H < r_V) // 왼쪽 블록
    {
        for (i = 0; i < block_size; i++)
        {
            for (j = 0; j < block_size; j++)
            {
                block[i][j] = block_H[i][j];
            }
        }
        return 0;
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

void Inverse_Quantization(int block[][block_size], char type)
{
    int i, j, k, q;
    if (type == 'e') // 인코딩인 경우
    {
        q = quantization_number;
    }
    else if (type == 'd') // 디코딩인 경우
    {
        q = quantization_number_decoding;
    }
    for (i = 0; i < block_size; i++)
    {
        for (j = 0; j < block_size; j++)
        {
            k = block[i][j] * q;
            if (k > 255) block[i][j] = 255;
            else block[i][j] = k;
        }
    }
}

void Recon(int block[][block_size], int t, int b_c)
{
    int i, j, k;

    if (t == 1) // 위쪽
    {
        for (i = 0; i < block_size; i++)
        {
            for (j = 0; j < block_size; j++)
            {
                k = block[j][i] + block_PV[j];
                if (k < 0)  block[j][i] = 0;
                else if (k > 255) block[j][i] = 255;
                else block[j][i] = k;


            }
        }
    }
    else if (t == 0) // 왼쪽
    {
        for (i = 0; i < block_size; i++)
        {
            for (j = 0; j < block_size; j++)
            {
                k = block[i][j] + block_PH[i];
                if (k < 0) block[i][j] = 0;
                else if (k > 255) block[i][j] = 255;
                else block[i][j] = k;

            }
        }
    }
}

void Recon_decoding(int block[][block_size], int b_c, int f, int t, unsigned char** recon_d)
{
    int i, j, k;
    int block_P[block_size][block_size];
    if (t == 1) // 위쪽
    {
        if (b_c < 22) // 위쪽 128
        {
           // printf("위쪽 128\n");
            for (i = 0; i < block_size; i++)
            {
                for (j = 0; j < block_size; j++)
                {
                    k = block[i][j] + prediction_number;
                    if (k < 0) block[i][j] = 0;
                    else if (k > 255) block[i][j] = 255;
                    else block[i][j] = k;

                }
            }
        }

        else // 위에 블록으로 recon
        {
            partition_recon(f, b_c - 22, block_P, recon_d);

            for (i = 0; i < block_size; i++)
            {
                for (j = 0; j < block_size; j++)
                {
                    k = block[j][i] + block_P[15][j];
                    if (k < 0) block[j][i] = 0;
                    else if (k > 255) block[j][i] = 255;
                    else block[j][i] = k;
                }
            }
        }
    }

    else if (t == 0) // 왼쪽
    {

        if (b_c % 22 == 0) // 왼쪽 128
        {
            for (i = 0; i < block_size; i++)
            {
                for (j = 0; j < block_size; j++)
                {
                    k = block[i][j] + prediction_number;
                    if (k < 0) block[i][j] = 0;
                    else if (k > 255) block[i][j] = 255;
                    else block[i][j] = k;
                }
            }
        }

        else // 옆에 블록으로 recon
        {
            partition_recon(f, b_c - 1, block_P, recon_d);

            for (i = 0; i < block_size; i++)
            {
                for (j = 0; j < block_size; j++)
                {
                    k = block[i][j] + block_P[i][15];
                    if (k < 0) block[i][j] = 0;
                    else if (k > 255) block[i][j] = 255;
                    else block[i][j] = k;

                }
            }
        }
    }

}

double PSNR(unsigned char** recon)
{

    double mse = 0, psnr = 0;
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
    return (psnr / number_frame);

}

void encoding(FILE* pOrg, FILE* pRecon_e, FILE* pBits_e)
{
    int i, j, b_c = 0;
    int t = -1;
    unsigned char** recon_e;


    recon_e = (unsigned char**)malloc(sizeof(unsigned char*) * number_frame);

    for (i = 0; i < number_frame; i++)
    {
        recon_e[i] = (unsigned char*)malloc(sizeof(unsigned char) * resoulution_size);
    }

    for (i = 0; i < number_frame; i++)
    {
        fread(org[i], 1, resoulution_size, pOrg);
    }

    for (i = 0; i < number_frame; i++) // 회색
    {
        for (j = width * height; j < resoulution_size; j++)
        {
            recon_e[i][j] = 128;
        }
    }

    for (int f = 0; f < number_frame; f++) // 프레임
    {
        bb = 1;

        for (b_c =0; b_c < block_hcount * block_wcount; b_c++) // 16*16 블록 
        {
            partition(f, b_c, block);
            t = Prediction(block, b_c, f, recon_e);
            Quantization(block);
            write_bitstream(f, block, Bitstream, t);
            Inverse_Quantization(block, 'e');
            Recon(block, t, b_c);
            write(f, b_c, block, recon_e);
        }


        fwrite(recon_e[f], 1, resoulution_size, pRecon_e);
        fwrite(Bitstream[f], 1, width * height + block_hcount * block_wcount +1, pBits_e);
    }
    printf("%.3f\n", PSNR(recon_e));
}

void decoding(FILE* pRecon_d)
{
    int i, j, b_c = 0;

    int t = -1;
    FILE* pBits_d = NULL;
    pBits_d = fopen("./bitstream.txt", "rb");

    unsigned char** recon_d;
    recon_d = (unsigned char**)malloc(sizeof(unsigned char*) * number_frame);

    for (i = 0; i < number_frame; i++)
    {
        recon_d[i] = (unsigned char*)malloc(sizeof(unsigned char) * resoulution_size);
    }


    for (i = 0; i < number_frame; i++)
    {
        fread(Bitstream[i], 1, width * height + block_hcount * block_wcount + 1, pBits_d);
    }


    for (i = 0; i < number_frame; i++) // 회색
    {
        for (j = width * height; j < resoulution_size; j++)
        {
            recon_d[i][j] = 128;
        }
    }

    for (int f = 0; f < number_frame; f++) // 프레임
    {
        bb = 1;

        for (b_c = 0; b_c < block_hcount * block_wcount; b_c++) // 16*16 블록 
        {
            t = read_bitstream(f, block, Bitstream);
            Inverse_Quantization(block, 'd');
            Recon_decoding(block, b_c, f, t, recon_d);
            write(f, b_c, block, recon_d);
        }

        fwrite(recon_d[f], 1, resoulution_size, pRecon_d);
    }
    printf("%.3f\n", PSNR(recon_d));


}

int main()
{
    int c = 1, b_c = 0, b_c_h = 0, k = 0;
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