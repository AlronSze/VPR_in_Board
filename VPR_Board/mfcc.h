#ifndef MFCC_H
#define MFCC_H

#include <QObject>

typedef struct frameData {
    double data;
    double real;
    double imag;
} FRAME_DATA;

typedef struct frame {
    FRAME_DATA *voiceData;
} FRAME;

class MFCC : public QObject
{
    Q_OBJECT
public:
    explicit MFCC(QObject *parent = 0);
    
signals:
    
public slots:
    
private:
    double GetOneFrameEnergy(FRAME_DATA *pFrameData, int pFrameLen);
    void BitReversed(FRAME_DATA *pFrameData, int pFrameLen);
    void PreEmphasis(double *pOriVoice, double *pEmpVoice, int pSampleSize);
    void FrameBlocking(double *pEmpVoice, FRAME *pFrameVoice, int pFrameNum, int pFrameLen, int pHalfFrameLen);
    void HammingWindow(double *pFrameData, int pFrameLen, int index);
    void FFT(FRAME_DATA *pFrameData, int pFrameLen, int pFFTOn);
    double CalLogEnergy(FRAME_DATA *voiceData, int pFrameLen, int pFilterIndex);
    double DCT(double *pSm, int pOrder);
    double GetDeltas(double *pCn, int pOrder);
    void GetMFCC(double *pOriVoice, int pSampleSize);

    bool m_GMM_result;
    bool m_GMM_option;

public:
    bool StartMFCC(bool pOption);
};

#endif // MFCC_H
