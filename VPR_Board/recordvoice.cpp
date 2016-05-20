#include "recordvoice.h"

#include <QDebug>
#include <QTimer>
#include <unistd.h>
#include <fcntl.h>
// #include <sys/ioctl.h>        // For board
// #include <linux/soundcard.h>  // For board

#define LENGTH    5
#define RATE      8000
#define DEEPTH    16
#define CHANNELS  1

RecordVoice::RecordVoice()
{
}

void RecordVoice::init_header(void)
{
    m_wave_head.a[0] = 'R';
    m_wave_head.a[1] = 'I';
    m_wave_head.a[2] = 'F';
    m_wave_head.a[3] = 'F';
    m_wave_head.b    = LENGTH * RATE * CHANNELS * DEEPTH / 8 - 8;
    m_wave_head.c[0] = 'W';
    m_wave_head.c[1] = 'A';
    m_wave_head.c[2] = 'V';
    m_wave_head.c[3] = 'E';
    m_wave_head.d[0] = 'f';
    m_wave_head.d[1] = 'm';
    m_wave_head.d[2] = 't';
    m_wave_head.d[3] = ' ';
    m_wave_head.e    = 16;
    m_wave_head.f    = 1;
    m_wave_head.g    = CHANNELS;
    m_wave_head.h    = RATE;
    m_wave_head.i    = RATE * CHANNELS * DEEPTH / 8;
    m_wave_head.j    = CHANNELS * DEEPTH / 8;
    m_wave_head.k    = DEEPTH;
    m_wave_head.p[0] = 'd';
    m_wave_head.p[1] = 'a';
    m_wave_head.p[2] = 't';
    m_wave_head.p[3] = 'a';
    m_wave_head.q    = LENGTH * RATE * CHANNELS * DEEPTH / 8;
}

void RecordVoice::init_record(void)
{
    init_header();
}

void RecordVoice::start_record(void)
{
    /* For board
    int i, arg;

    m_fd_dev_r= open("/dev/dsp", O_RDONLY, 0777);
    if (m_fd_dev_r < 0)
    {
        perror("Cannot open /dev/dsp device");
    }

    arg = DEEPTH;
    if (ioctl(m_fd_dev_r, SOUND_PCM_WRITE_BITS, &arg) == -1)
    {
        perror("Cannot set SOUND_PCM_WRITE_BITS ");
    }

    arg = CHANNELS;
    if (ioctl(m_fd_dev_r, SOUND_PCM_WRITE_CHANNELS, &arg) == -1)
    {
        perror("Cannot set SOUND_PCM_WRITE_CHANNELS");
    }
    arg = RATE;
    if (ioctl(m_fd_dev_r, SOUND_PCM_WRITE_RATE, &arg) == -1)
    {
        perror("Cannot set SOUND_PCM_WRITE_WRITE");
    }

    if((m_fd_f = open("/VPR/sound.wav", O_CREAT|O_TRUNC|O_RDWR, 0777)) == -1)
    {
        perror("cannot creat the sound file");
    }
    if(write(m_fd_f, &m_wave_head, sizeof(m_wave_head)) == -1)
    {
       perror("write to sound'head wrong!!");
    }

    for(i = 0; i < (m_wave_head.q / BUFSIZE); i++)
    {
        if (read(m_fd_dev_r, m_buf, sizeof(m_buf)) != sizeof(m_buf))
        {
            perror("read wrong number of bytes");
        }
        if (write(m_fd_f, m_buf, sizeof(m_buf)) == -1)
        {
            perror("write to sound wrong");
        }
    }

    close(m_fd_f);
    close(m_fd_dev_r);
    */
}

void RecordVoice::play_record(void)
{
    /* For board
    int i, arg;

    m_fd_dev_w = open("/dev/dsp", O_WRONLY, 0777);
    if (m_fd_dev_w < 0)
    {
        perror("Cannot open /dev/dsp device");
    }

    arg = DEEPTH;
    if (ioctl(m_fd_dev_w, SOUND_PCM_WRITE_BITS, &arg) == -1)
    {
       perror("Cannot set SOUND_PCM_WRITE_BITS ");
    }
    arg = CHANNELS;
    if (ioctl(m_fd_dev_w, SOUND_PCM_WRITE_CHANNELS, &arg) == -1)
    {
        perror("Cannot set SOUND_PCM_WRITE_CHANNELS");
    }
    arg = RATE;
    if (ioctl(m_fd_dev_w, SOUND_PCM_WRITE_RATE, &arg) == -1)
    {
        perror("Cannot set SOUND_PCM_WRITE_WRITE");
    }

    if((m_fd_f = open("/VPR/sound.wav", O_RDONLY, 0777)) == -1)
    {
        perror("cannot creat the sound file");
    }

    lseek(m_fd_f, 44, SEEK_SET);
    for(i = 0; i < (m_wave_head.q / BUFSIZE); i++)
    {
        if (read(m_fd_f, m_buf, sizeof(m_buf)) != sizeof(m_buf))
        {
            perror("read wrong number of buf");
        }
        if (write(m_fd_dev_w, m_buf, sizeof(m_buf)) != sizeof(m_buf))
        {
            perror("write to dev wrong");
        }
    }

    close(m_fd_f);
    close(m_fd_dev_w);
    */
}
