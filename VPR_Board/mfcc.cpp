#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mfcc.h"
#include "gmm.h"

#define FILTER_NUM 24
#define DCT_ORDER  13
#define DELTA_N    2

MFCC::MFCC(QObject *parent) :
    QObject(parent)
{
}

static int sMel_24_256_8000[FILTER_NUM + 2] = {
    9,
    12,  15,  19,  22,  27,  31,
    36,  41,  47,  53,  60,  68,
    76,  85,  95,  105, 117, 130,
    143, 158, 175, 193, 212, 233,
    256
};

double MFCC::GetOneFrameEnergy(FRAME_DATA *pFrameData, int pFrameLen)
{
    int i;
    double energy = 0.0;
    for (i = 0; i < pFrameLen; i++)
    {
        energy += pFrameData[i].data * pFrameData[i].data;
    }

    energy = log10(energy) * 10.0;
    return energy;
}

void MFCC::PreEmphasis(double *pOriVoice, double *pEmpVoice, int pSampleSize)
{
    int i;

    pEmpVoice[0] = pOriVoice[0];

    for (i = 1; i < pSampleSize; i++)
    {
        pEmpVoice[i] = pOriVoice[i] - 0.97 * pOriVoice[i - 1];
    }
}

void MFCC::FrameBlocking(double *pEmpVoice, FRAME *pFrameVoice, int pFrameNum, int pFrameLen, int pHalfFrameLen)
{
    int i, j, k;
    double oneFrameEnergy;
    double sm[FILTER_NUM];
    double cn[DCT_ORDER];
    double dt1[DCT_ORDER - 4];
    double dt2[DCT_ORDER - 8];
    double mfcc[pFrameNum][DCT_ORDER + (DCT_ORDER - 4) + (DCT_ORDER - 8) + 1];

    for (i = 0; i < pFrameNum; i++)
    {
        pFrameVoice[i].voiceData = (FRAME_DATA *)malloc(sizeof(FRAME_DATA)* pFrameLen);

        for (j = 0; j < pFrameLen; j++)
        {
            pFrameVoice[i].voiceData[j].imag = 0.0;
            pFrameVoice[i].voiceData[j].real = pEmpVoice[(i * pHalfFrameLen) + j];
            HammingWindow(&pFrameVoice[i].voiceData[j].real, pFrameLen, j);
        }

        FFT(pFrameVoice[i].voiceData, pFrameLen, 1);

        oneFrameEnergy = GetOneFrameEnergy(pFrameVoice[i].voiceData, pFrameLen);

        for (k = 0; k < FILTER_NUM; k++)
        {
            sm[k] = CalLogEnergy(pFrameVoice[i].voiceData, pFrameLen, k + 1);
        }

        for (k = 0; k < DCT_ORDER; k++)
        {
            cn[k] = DCT(sm, k);
        }

        for (k = 0; k < DCT_ORDER - 4; k++)
        {
            dt1[k] = GetDeltas(cn, k + 2);
        }

        for (k = 0; k < DCT_ORDER - 8; k++)
        {
            dt2[k] = GetDeltas(dt1, k + 2);
        }

        for (k = 0; k < (DCT_ORDER + (DCT_ORDER - 4) + (DCT_ORDER - 8)); k++)
        {
            if (k < DCT_ORDER)
            {
                mfcc[i][k] = cn[k];
            }
            else if (k < (DCT_ORDER + (DCT_ORDER - 4)))
            {
                mfcc[i][k] = dt1[k - DCT_ORDER];
            }
            else if (k < (DCT_ORDER + (DCT_ORDER - 4) + (DCT_ORDER - 8)))
            {
                mfcc[i][k] = dt2[k - DCT_ORDER - (DCT_ORDER - 4)];
            }
        }
        mfcc[i][k] = oneFrameEnergy;
    }

    GMM gmm;
    m_GMM_result = gmm.startGMM(mfcc, pFrameNum, m_GMM_option);
}

void MFCC::HammingWindow(double *pFrameData, int pFrameLen, int index)
{
    double W = (1.0 - 0.46) - 0.46 * cos((2 * M_PI * (double)index) / ((double)pFrameLen - 1.0));
    *pFrameData *= W;
}

void MFCC::BitReversed(FRAME_DATA *pFrameData, int pFrameLen)
{
    double realTemp;
    int i, j, k;
    for (i = 1, j = pFrameLen / 2; i < pFrameLen - 1; i++)
    {
        if (i < j)
        {
            realTemp = pFrameData[i].real;
            pFrameData[i].real = pFrameData[j].real;
            pFrameData[j].real = realTemp;
        }

        k = pFrameLen / 2;

        while (j >= k)
        {
            j -= k;
        }

        j += k;
    }
}

void MFCC::FFT(FRAME_DATA *pFrameData, int pFrameLen, int pFFTOn)
{
    int i, j, k;

    BitReversed(pFrameData, pFrameLen);

    for (i = 2; i <= pFrameLen; i <<= 1)
    {
        FRAME_DATA wn;
        wn.real = cos(-pFFTOn * 2 * M_PI / i);
        wn.imag = sin(-pFFTOn * 2 * M_PI / i);

        for (j = 0; j < pFrameLen; j += i)
        {
            FRAME_DATA w;
            w.real = 1.0;
            w.imag = 0.0;

            for (k = j; k < j + i / 2; k++)
            {
                FRAME_DATA u, t, wTemp;
                u.real = pFrameData[k].real;
                u.imag = pFrameData[k].imag;
                t.real = w.real * pFrameData[k + i / 2].real - w.imag * pFrameData[k + i / 2].imag;
                t.imag = w.real * pFrameData[k + i / 2].imag + w.imag * pFrameData[k + i / 2].real;

                pFrameData[k].real = u.real + t.real;
                pFrameData[k].imag = u.imag + t.imag;
                pFrameData[k].data = pFrameData[k].real * pFrameData[k].real + pFrameData[k].imag * pFrameData[k].imag;

                pFrameData[k + i / 2].real = u.real - t.real;
                pFrameData[k + i / 2].imag = u.imag - t.imag;
                pFrameData[k + i / 2].data = pFrameData[k + i / 2].real * pFrameData[k + i / 2].real + pFrameData[k + i / 2].imag * pFrameData[k + i / 2].imag;

                wTemp.real = w.real;
                wTemp.imag = w.imag;
                w.real = wTemp.real * wn.real - wTemp.imag * wn.imag;
                w.imag = wTemp.real * wn.imag - wTemp.imag * wn.real;
            }
        }
    }

    if (pFFTOn == -1)
    {
        for (i = 0; i < pFrameLen; i++)
        {
            pFrameData[i].real /= pFrameLen;
        }
    }
}

double MFCC::CalLogEnergy(FRAME_DATA *voiceData, int pFrameLen, int pFilterIndex)
{
    int k;
    double hm;
    double total = 0.0;
    for (k = 0; k < pFrameLen; k++)
    {
        if (k < sMel_24_256_8000[pFilterIndex - 1])
        {
            hm = 0;
        }
        else if ((k >= sMel_24_256_8000[pFilterIndex - 1]) && (k <= sMel_24_256_8000[pFilterIndex]))
        {
            hm = (double)(k - sMel_24_256_8000[pFilterIndex - 1]) / (double)(sMel_24_256_8000[pFilterIndex] - sMel_24_256_8000[pFilterIndex - 1]);
        }
        else if ((k >= sMel_24_256_8000[pFilterIndex]) && (k <= sMel_24_256_8000[pFilterIndex + 1]))
        {
            hm = (double)(sMel_24_256_8000[pFilterIndex + 1] - k) / (double)(sMel_24_256_8000[pFilterIndex + 1] - sMel_24_256_8000[pFilterIndex]);
        }
        else
        {
            hm = 0;
        }

        total += voiceData[k].data * hm;
    }

    return log(total);
}

double MFCC::DCT(double *pSm, int pOrder)
{
    int i;
    double c0  = 0.0;
    double c1n = 0.0;
    double cn;
    for (i = 0; i < FILTER_NUM; i++)
    {
        if (i == 0)
        {
            c0 = pSm[i];
        }
        else
        {
            c1n += pSm[i] * cos((M_PI * (2.0 * (double)pOrder + 1) * i) / (2.0 * (double)FILTER_NUM));
        }
    }

    cn = sqrt(1.0 / (double)FILTER_NUM) * c0 + sqrt(2.0 / (double)FILTER_NUM) * c1n;

    return cn;
}

double MFCC::GetDeltas(double *pCn, int pOrder)
{
    int i;
    double numer = 0.0, denom = 0.0;
    double dt;

    for (i = 1; i <= DELTA_N; i++)
    {
        numer += (double)i * (pCn[pOrder + i] - pCn[pOrder - i]);
        denom += (double)i * (double)i;
    }

    dt = numer / (2.0 * denom);

    return dt;
}

void MFCC::GetMFCC(double *pOriVoice, int pSampleSize)
{
    int frameLen = 256;
    int halfFrameLen = frameLen / 2;
    int frameNum = pSampleSize / halfFrameLen - 1;
    FRAME *frameVoice;
    double *empVoice;

    empVoice = (double *)malloc(sizeof(double) * pSampleSize);

    PreEmphasis(pOriVoice, empVoice, pSampleSize);

    frameVoice = (FRAME *)malloc(sizeof(FRAME)* frameNum);
    FrameBlocking(empVoice, frameVoice, frameNum, frameLen, halfFrameLen);

    free(frameVoice);
}

bool MFCC::StartMFCC(bool pOption)
{
    double *test;
    char tempLow, tempHigh;
    FILE *file;
    int fileLen, inputLen;
    int i;

    m_GMM_option = pOption;

    if ((file = fopen("/VPR/sound.wav", "rb")) == NULL)
    {
        printf ("open error!");
    }

    fseek(file, 0, SEEK_END);
    fileLen = ftell(file) - 44;
    fseek(file, 44, SEEK_SET);

    inputLen = (fileLen / 2) - ((fileLen / 2) % 256);

    test = (double *)malloc(sizeof(double) * inputLen);

    for (i = 0; i < inputLen; i++)
    {
        fread(&tempLow, sizeof(unsigned char), 1, file);
        fread(&tempHigh, sizeof(unsigned char), 1, file);
        test[i] = (double)(short)(tempLow | (tempHigh << 8));
        if (test[i] == 0.0)
        {
            test[i] = 1.0;
        }
    }

    GetMFCC(test, inputLen);
    free(test);

    if (!m_GMM_result) {
        return false;
    }

    return true;
}
