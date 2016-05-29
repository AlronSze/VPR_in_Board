#include "gmm.h"
#include <QFile>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

GMM::GMM(QObject *parent) : QObject(parent)
{

}

void *GMM::CallocGMM(GMM_STRUCT * pGMM)
{
    int i;
    int m = pGMM->m;

    pGMM->p = (double *)calloc(m, sizeof(double));
    if (!pGMM->p)
    {
        return NULL;
    }
    pGMM->u = (double **)malloc(m * sizeof(double *));
    if (!pGMM->u)
    {
        free(pGMM->p);
        return NULL;
    }
    pGMM->cMatrix = (double **)malloc(m * sizeof(double *));
    if (!pGMM->cMatrix)
    {
        free(pGMM->p);
        free(pGMM->u);
        return NULL;
    }
    for (i = 0; i < m; ++i)
    {
        pGMM->u[i] = (double *)calloc(D, sizeof(double));
        if (!pGMM->u[i])
        {
            for (--i; i >= 0; --i)
            {
                free(pGMM->u[i]);
                free(pGMM->cMatrix[i]);
            }
            free(pGMM->p);
            free(pGMM->u);
            free(pGMM->cMatrix);
            return NULL;
        }
        pGMM->cMatrix[i] = (double *)calloc(D, sizeof(double));
        if (!pGMM->cMatrix[i])
        {
            free(pGMM->u[i]);
            for (--i; i >= 0; --i)
            {
                free(pGMM->u[i]);
                free(pGMM->cMatrix[i]);
            }
            free(pGMM->p);
            free(pGMM->u);
            free(pGMM->cMatrix);
            return NULL;
        }
    }
    return pGMM;
}

void GMM::FreeGMM(GMM_STRUCT * pGMM)
{
    int i;
    int m = pGMM->m;

    if (!pGMM)
    {
        return;
    }
    if (pGMM->p)
    {
        free(pGMM->p);
    }
    if (!pGMM->u && !pGMM->cMatrix)
    {
        return;
    }
    for (i = 0; i < m; ++i)
    {
        if (pGMM->u && pGMM->u[i])
        {
            free(pGMM->u[i]);
        }
        if (pGMM->cMatrix && pGMM->cMatrix[i])
        {
            free(pGMM->cMatrix[i]);
        }
    }
    if (pGMM->u)
    {
        free(pGMM->u);
    }
    if (pGMM->cMatrix)
    {
        free(pGMM->cMatrix);
    }
}

void GMM::InitGMMStruct(GMM_STRUCT * pGMM, int m)
{
    pGMM->m = m;
    pGMM->p = NULL;
    pGMM->u = NULL;
    pGMM->cMatrix = NULL;
}

int GMM::InitGMM(double X[][D], GMM_STRUCT * pGMM, int train_frame_num, int m)
{
    int i;
    int j;
    int * clusterIndex = NULL;
    int * clusterSize = NULL;
    double ** sum = NULL;
    GMM_STRUCT gmm = *pGMM;


    clusterIndex = (int *)calloc(train_frame_num, sizeof(int));
    clusterSize = (int *)calloc(m, sizeof(int));
    sum = (double **)malloc(m * sizeof(double *));
    for (i = 0; i < m; ++i)
    {
        sum[i] = (double *)calloc(D, sizeof(double));
    }

    k_means(X, pGMM, clusterIndex, clusterSize, train_frame_num, m);

    for (i = 0; i < m; ++i)
    {
        gmm.p[i] = 1.0 / (double)m;
    }


    for (i = 0; i < train_frame_num; ++i)
    {
        for (j = 0; j < D; ++j)
        {
            sum[clusterIndex[i]][j] += pow(X[i][j], 2);
        }
    }
    for (i = 0; i < m; ++i)
    {
        if (clusterSize[i] != 0)
        {
            for (j = 0; j < D; ++j)
            {
                gmm.cMatrix[i][j] = sum[i][j] / clusterSize[i] - pow(gmm.u[i][j], 2);
                if (gmm.cMatrix[i][j] < 0)
                {
                    FreeGMM(&gmm);
                    free(clusterIndex);
                    free(clusterSize);
                    for (i = 0; i < m; ++i)
                    {
                        free(sum[i]);
                    }
                    free(sum);
                    return 0;
                }
                else if (gmm.cMatrix[i][j] < 1e-10)
                {
                    gmm.cMatrix[i][j] = 0;
                }
            }
        }
        else
        {
            FreeGMM(&gmm);
            free(clusterIndex);
            free(clusterSize);
            for (i = 0; i < m; ++i)
            {
                free(sum[i]);
            }
            free(sum);
            return 0;
        }
    }

    free(clusterIndex);
    free(clusterSize);
    for (i = 0; i < m; ++i)
    {
        free(sum[i]);
    }
    free(sum);
    return 1;
}

double GMM::GMM_diff(GMM_STRUCT * gmm1, GMM_STRUCT * gmm2)
{
    int i;
    int j;
    double diff = 0;

    for (i = 0; i < M; ++i)
    {
        diff += pow(gmm1->p[i] - gmm2->p[i], 2);
        for (j = 0; j < D; ++j)
        {
            diff += pow(gmm1->u[i][j] - gmm2->u[i][j], 2);
            diff += pow(gmm1->cMatrix[i][j] - gmm2->cMatrix[i][j], 2);
        }
    }

    return diff;
}

double GMM::getDistance(double * p1, double * p2)
{
    int i = 0;
    double distance = 0;

    while (i < D)
    {
        distance += pow(p1[i] - p2[i], 2);
        ++i;
    }
    return distance;
}

void GMM::k_means(double c_vector[][D], GMM_STRUCT * pGMM, int * clusterIndex, int * cSize, int train_frame_num, int m)
{
    int i;
    int j;
    int k;
    int farIndex = 0;
    int * clusterSize = NULL;
    double maxDistance = 0;
    double minDistance = 0;
    double distance = 0;
    double ** mCenter = NULL;
    double diff_Center = 0;
    GMM_STRUCT gmm = *pGMM;

    clusterSize = (int *)calloc(m, sizeof(int));
    mCenter = (double **)malloc(m * sizeof(double *));
    for (i = 0; i < m; ++i)
    {
        mCenter[i] = (double *)calloc(D, sizeof(double));
    }

    for (i = 0; i < D; ++i)
    {
        for (j = 0; j < train_frame_num; ++j)
        {
            gmm.u[0][i] += c_vector[j][i];
        }
        gmm.u[0][i] /= train_frame_num;
    }

    for (k = 1; k < m; ++k)
    {
        farIndex = 0;
        maxDistance = 0;

        for (j = 0; j < train_frame_num; ++j)
        {
            distance = 0;
            for (i = 0; i < k; ++i)
            {
                distance += sqrt(getDistance(gmm.u[i], c_vector[j]));
            }
            if (distance > maxDistance)
            {
                maxDistance = distance;
                farIndex = j;
            }
        }
        memcpy(gmm.u[k], c_vector[farIndex], D*sizeof(double));

NEXT:
        for (j = 0; j < k + 1; ++j)
        {
            memcpy(mCenter[j], gmm.u[j], D*sizeof(double));
        }

        for (j = 0; j < train_frame_num; ++j)
        {
            farIndex = 0;
            minDistance = getDistance(mCenter[0], c_vector[j]);
            for (i = 1; i <= k; ++i)
            {
                distance = getDistance(mCenter[i], c_vector[j]);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    farIndex = i;
                }
            }

            if (k == m - 1)
            {
                clusterIndex[j] = farIndex;
            }
            clusterSize[farIndex] += 1;
            for (i = 0; i < D; ++i)
            {
                if (clusterSize[farIndex] == 1)
                {
                    gmm.u[farIndex][i] = 0;
                }
                gmm.u[farIndex][i] += c_vector[j][i];
            }

        }

        if (k == m - 1)
        {
            memcpy(cSize, clusterSize, m * sizeof(int));
        }

        diff_Center = 0;
        for (i = 0; i <= k; ++i)
        {
            double temp = 0;

            if (clusterSize[i] > 0)
            {
                for (j = 0; j < D; ++j)
                {
                    gmm.u[i][j] /= clusterSize[i];
                    temp += pow(gmm.u[i][j] - mCenter[i][j], 2);
                }
                diff_Center += sqrt(temp);
            }
            clusterSize[i] = 0;
        }
        if (diff_Center >= DIFF_CENTER_VALUE)
        {
            goto NEXT;
        }

    }

    free(clusterSize);
    for (i = 0; i < m; ++i)
    {
        free(mCenter[i]);
    }
    free(mCenter);
}

double GMM::GMM_density(GMM_STRUCT * pGMM, double * X, int index)
{
    int i;
    double temp = 0;
    double sqrt_Matrix_value = 1;
    double res = 0;


    for (i = 0; i < D; ++i)
    {
        temp += pow(X[i] - pGMM->u[index][i], 2) / pGMM->cMatrix[index][i];
        sqrt_Matrix_value *= sqrt(pGMM->cMatrix[index][i]);
    }
    res = pow(_2PI, D / -2) / sqrt_Matrix_value * exp(temp / -2);
    return res;
}

int GMM::BuildGMM(GMM_STRUCT * pGMM, GMM_STRUCT * Out, double X[][D], int train_frame_num, int m)
{
    int i;
    int j;
    int k;
    GMM_STRUCT temp_gmm = *Out;
    double ** p = NULL;
    double * sum_p = NULL;
    double * pb = NULL;
    double sum_pb = 0;

    pb = (double *)calloc(m, sizeof(double));
    sum_p = (double *)calloc(m, sizeof(double));
    p = (double **)malloc(train_frame_num * sizeof(double *));
    for (i = 0; i < train_frame_num; ++i)
    {
        p[i] = (double *)calloc(m, sizeof(double));
    }

    for (i = 0; i < train_frame_num; ++i)
    {
        sum_pb = 0;
        for (j = 0; j < m; ++j)
        {
            pb[j] = pGMM->p[j] * GMM_density(pGMM, X[i], j);
            sum_pb += pb[j];
        }
        if (sum_pb == 0 || isnan(sum_pb))
        {
            goto ERROR_RETURN;
        }
        for (j = 0; j < m; ++j)
        {
            p[i][j] = pb[j] / sum_pb;
            sum_p[j] += p[i][j];
        }
    }

    for (j = 0; j < m; ++j)
    {
        temp_gmm.p[j] = sum_p[j] / train_frame_num;
        for (i = 0; i < train_frame_num; ++i)
        {
            for (k = 0; k < D; ++k)
            {
                if (i == 0)
                {
                    temp_gmm.u[j][k] = 0;
                    temp_gmm.cMatrix[j][k] = 0;
                }
                temp_gmm.u[j][k] += p[i][j] * X[i][k];
                temp_gmm.cMatrix[j][k] += p[i][j] * pow(X[i][k], 2);
            }
        }
        for (k = 0; k < D; ++k)
        {
            temp_gmm.u[j][k] /= sum_p[j];
            temp_gmm.cMatrix[j][k] /= sum_p[j];
            temp_gmm.cMatrix[j][k] -= pow(temp_gmm.u[j][k], 2);
            if (temp_gmm.cMatrix[j][k] <= 0)
            {
ERROR_RETURN:
                for (i = 0; i < train_frame_num; ++i)
                {
                    free(p[i]);
                }
                free(p);
                free(pb);
                free(sum_p);
                return 0;
            }
        }
    }

    for (i = 0; i < train_frame_num; ++i)
    {
        free(p[i]);
    }
    free(p);
    free(pb);
    free(sum_p);
    return 1;
}

int GMM::GMM_process(double X[][D], GMM_STRUCT * pGMM, int train_frame_num, int m)
{
    GMM_STRUCT first_gmm;
    GMM_STRUCT second_gmm;
    GMM_STRUCT *first = &first_gmm;
    GMM_STRUCT *second = &second_gmm;
    GMM_STRUCT *temp = NULL;
    int bRes;
    double diff = 0;

    InitGMMStruct(&first_gmm, m);
    CallocGMM(&first_gmm);
    InitGMMStruct(&second_gmm, m);
    CallocGMM(&second_gmm);

    if (!InitGMM(X, first, train_frame_num, m))
    {
        return 0;
    }

    do {
        bRes = BuildGMM(first, second, X, train_frame_num, m);
        if (!bRes)
        {
            FreeGMM(first);
            FreeGMM(second);
            return 0;
        }
        temp = first;
        first = second;
        second = temp;
        diff = GMM_diff(first, second);
    } while (diff >= DIFF_GMM_VALUE);

    FreeGMM(first);
    *pGMM = *second;
    return 1;
}

int GMM::GMM_identify(double X[][D], double * value, GMM_STRUCT * pGMM, int frame_num, int m)
{
    int j;
    int k;
    double temp = 0;
    double * pb = NULL;
    double sum_pb = 0;


    pb = (double *)calloc(m, sizeof(double));
    for (j = 0; j < frame_num; ++j)
    {
        sum_pb = 0;
        for (k = 0; k < m; ++k)
        {
            pb[k] = pGMM[0].p[k] * GMM_density(pGMM, X[j], k);
            sum_pb += pb[k];
        }
        if (sum_pb <= 0)
        {
            return 0;
        }
        else
        {
            temp += log(sum_pb);
        }
    }

    *value = temp;
    free(pb);
    return 1;
}

int GMM::char_to_int(QByteArray pChar)
{
    int num = pChar.size();
    int result_int;
    char result_char[32];

    for (int i = 0; i < num; i++)
    {
        result_char[i] = pChar.at(i);
    }
    result_int = atoi(result_char);

    return result_int;
}

void GMM::GMM_set_file(double pResult)
{
    QString file_name = "/VPR/file/gmm_id";
    char result_str[32];
    if (QFile::exists(file_name))
    {
        QFile::remove(file_name);
    }
    QFile file(file_name);
    file.open(QIODevice::WriteOnly);
    sprintf(result_str, "%d@", (int)pResult);
    file.write(result_str);
    file.close();
}

double GMM::GMM_get_file(void)
{
    QString file_name = "/VPR/file/gmm_id";
    QFile file(file_name);
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();
    file.close();

    int count, result;
    for (count = 0; count < file_data.size(); count++)
    {
        if (file_data.at(count) == '@')
        {
            break;
        }
    }
    file_data.resize(count + 2);
    result = char_to_int(file_data);

    return (double)result;
}

bool GMM::GMM_import_struct(void)
{
    QString file_name = "/VPR/file/gmm";
    if (!QFile::exists(file_name))
    {
        return false;
    }

    FILE *f_in;
    int temp_m;
    f_in = fopen("/VPR/file/gmm", "rb");
    fread(&temp_m, sizeof(temp_m), 1, f_in);
    InitGMMStruct(&m_gmm, temp_m);
    CallocGMM(&m_gmm);
    fread(m_gmm.p, sizeof(m_gmm.p[0]), m_gmm.m, f_in);
    for (int i = 0; i < m_gmm.m; i++)
    {
        fread(m_gmm.u[i], sizeof(m_gmm.u[0][0]), D, f_in);
    }
    for (int i = 0; i < m_gmm.m; i++)
    {
        fread(m_gmm.cMatrix[i], sizeof(m_gmm.cMatrix[0][0]), D, f_in);
    }
    fclose(f_in);

    return true;
}

void GMM::GMM_export_struct(void)
{
    QString file_name = "/VPR/file/gmm";
    if (QFile::exists(file_name))
    {
        QFile::remove(file_name);
    }

    FILE *f_out;
    f_out = fopen("/VPR/file/gmm", "wb");
    fwrite(&m_gmm.m, sizeof(m_gmm.m), 1, f_out);
    fwrite(m_gmm.p, sizeof(m_gmm.p[0]), m_gmm.m, f_out);
    for (int i = 0; i < m_gmm.m; ++i)
    {
        fwrite(m_gmm.u[i], sizeof(m_gmm.u[0][0]), D, f_out);
    }
    for (int i = 0; i < m_gmm.m; ++i)
    {
        fwrite(m_gmm.cMatrix[i], sizeof(m_gmm.cMatrix[0][0]), D, f_out);
    }
    fclose(f_out);
}

bool GMM::startGMM(double pMFCC[][D], int pNum, bool pOption)
{
    double result;

    if (pOption) {
        if (!GMM_process(pMFCC, &m_gmm, pNum, M))
        {
            return false;
        }
        GMM_identify(pMFCC, &result, &m_gmm, pNum, M);
        GMM_set_file(result);
        GMM_export_struct();
    }
    else {
        if (!GMM_import_struct())
        {
            return false;
        }
        GMM_identify(pMFCC, &result, &m_gmm, pNum, M);
        if (GMM_get_file() < (result * GMM_THRESHOLD))
        {
            return false;
        }
    }

    return true;
}
