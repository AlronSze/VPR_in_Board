#ifndef DATAPACKET_H
#define DATAPACKET_H

#include <QByteArray>

#define RECORD      0x11111111
#define REPLAY      0x22222222
#define LOGIN       0x33333333
#define REGISTER    0x44444444
#define SENDNAME    0x55555555
#define SENDFILE    0x66666666
#define LISTFILE    0x77777777
#define GETFILE     0x88888888
#define DELFILE     0x99999999
#define KEYEXCHANGE 0x10101010
#define EXITTASK    0x12121212

#define CONTINUE 0
#define ERROR    1
#define OVER     2

class DataPacket
{
public:
    DataPacket();

    unsigned int get_command(void);
    QByteArray get_byte_array(void);
    QByteArray pack_data(unsigned int command, QByteArray data_byte);
    unsigned char unpack_data(QByteArray bytes);

private:
    unsigned int m_command;
    unsigned int m_data_length;
    unsigned int m_data_check;
    unsigned int m_magic;
    QByteArray   m_packet_data;

    QByteArray   m_byte_buffer;
    unsigned int m_recv_pointer;
    unsigned int m_scan_pointer;

    QByteArray int_to_bytes(unsigned int number, QByteArray byte_array, int offset);
    unsigned int bytes_to_int(QByteArray bytes, int offset);
    void get_header(void);
};

#endif // DATAPACKET_H
