#ifndef RECORDVOICE_H
#define RECORDVOICE_H

#define BUFSIZE   8

class RecordVoice
{
public:
    RecordVoice();

    void init_header(void);
    void init_record(void);
    void start_record(void);
    void play_record(void);

private:
    struct WAVEHEAD {
        unsigned char a[4];
        long int b;
        unsigned char c[4];
        unsigned char d[4];
        long int e;
        short int f;
        short int g;
        long int h;
        long int i;
        short int j;
        short int k;
        unsigned char p[4];
        long int q;
    } m_wave_head;

    unsigned char m_buf[BUFSIZE];
    int m_fd_dev_r;
    int m_fd_dev_w;
    int m_fd_f;
};

#endif // RECORDVOICE_H
