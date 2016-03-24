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
    struct WAVEHEAD
    {
        /****RIFF WAVE CHUNK*/
        unsigned char a[4];//四个字节存放'R','I','F','F'
        long int b;        //整个文件的长度-8;每个Chunk的size字段，都是表示除了本Chunk的ID和SIZE字段外的长度;
        unsigned char c[4];//四个字节存放'W','A','V','E'
        /****RIFF WAVE CHUNK*/
        /****Format CHUNK*/
        unsigned char d[4];//四个字节存放'f','m','t',''
        long int e;        //16后没有附加消息，18后有附加消息；一般为16，其他格式转来的话为18
        short int f;       //编码方式，一般为0x0001;
        short int g;       //声道数目，1单声道，2双声道;
        long int h;        //采样频率;
        long int i;        //每秒所需字节数;
        short int j;       //每个采样需要多少字节，若声道是双，则两个一起考虑;
        short int k;       //即量化位数
        /****Format CHUNK*/
        /***Data Chunk**/
        unsigned char p[4];//四个字节存放'd','a','t','a'
        long int q;        //语音数据部分长度，不包括文件头的任何部分
    } m_wave_head;

    unsigned char m_buf[BUFSIZE];
    int m_fd_dev_r;
    int m_fd_dev_w;
    int m_fd_f;
};

#endif // RECORDVOICE_H
