#ifndef GMM_H
#define GMM_H

#include <QObject>

#define D                  28
#define M                  16
#define _2PI               6.2831853L
#define DIFF_GMM_VALUE     1
#define DIFF_CENTER_VALUE  1

typedef struct GMMStruct {
    int m;
    double *p;
    double **u;
    double **cMatrix;
} GMM_STRUCT;

class GMM : public QObject
{
    Q_OBJECT
public:
    explicit GMM(QObject *parent = 0);

signals:

public slots:

private:
    int InitGMM(double X[][D], GMM_STRUCT * pGMM, int train_frame_num, int m);
    void *CallocGMM(GMM_STRUCT * pGMM);
    void FreeGMM(GMM_STRUCT * pGMM);
    int GMM_process(double X[][D], GMMStruct *pGMM, int train_frame_num, int m);
    int GMM_identify(double X[][D], double *value, GMMStruct *pGMM, int frame_num, int m);
    int BuildGMM(GMM_STRUCT * pGMM, GMM_STRUCT * Out, double X[][D], int train_frame_num, int m);
    void InitGMMStruct(GMMStruct *pGMM, int m);
    void k_means(double c_vector[][D], GMMStruct *pGMM, int *clusterIndex, int *cSize, int train_frame_num, int m);
    double GMM_diff(GMM_STRUCT * gmm1, GMM_STRUCT * gmm2);
    double getDistance(double * p1, double * p2);
    double GMM_density(GMM_STRUCT * pGMM, double * X, int index);
public:
    bool startGMM(double pMFCC[][D], int pNum,bool pOption);
};

#endif // GMM_H
