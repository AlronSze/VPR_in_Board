#include "datapacket.h"

DataPacket::DataPacket(): m_command(0),
    m_data_length(0),
    m_data_check(0),
    m_magic(0),
    m_recv_pointer(0),
    m_scan_pointer(0)
{
    m_byte_buffer.resize(0);
}

unsigned int DataPacket::get_command(void)
{
    return m_command;
}

QByteArray DataPacket::get_byte_array(void)
{
    return m_packet_data;
}

QByteArray DataPacket::pack_data(unsigned int command, QByteArray data_byte)
{
    m_command = command;
    m_magic   = m_command & 0xFFFFFFFF;

    if (data_byte.isEmpty())
    {
        m_data_length = 0;
        m_data_check  = 0;

        QByteArray header;
        header.resize(16);
        header = int_to_bytes(m_command,     header, 0);
        header = int_to_bytes(m_data_length, header, 4);
        header = int_to_bytes(m_data_check,  header, 8);
        header = int_to_bytes(m_magic,       header, 12);

        return header;
    }

    m_data_length = data_byte.length();

    char *crc16_check = data_byte.data();
    m_data_check = (unsigned int)qChecksum(crc16_check, m_data_length);

    QByteArray packet;
    packet.resize(16 + m_data_length);

    packet = int_to_bytes(m_command,     packet, 0);
    packet = int_to_bytes(m_data_length, packet, 4);
    packet = int_to_bytes(m_data_check,  packet, 8);
    packet = int_to_bytes(m_magic,       packet, 12);
    for (unsigned int i = 0; i < m_data_length; i++)
    {
        packet[16 + i] = data_byte[i];
    }

    return packet;
}

unsigned char DataPacket::unpack_data(QByteArray bytes)
{
    int buf_length = bytes.length();

    m_byte_buffer.resize(m_byte_buffer.length() + buf_length);
    for (int i = 0; i < buf_length; i++, m_recv_pointer++)
    {
        m_byte_buffer[m_recv_pointer] = bytes[i];
    }

    get_header();

    if (m_scan_pointer >= 16)
    {
        if ((m_command & 0xFFFFFFFF) == m_magic)
        {
            if (m_data_length > 0)
            {
                if (m_recv_pointer > 16)
                {
                    if ((m_recv_pointer - m_scan_pointer) <= m_data_length)
                    {
                        for (; m_scan_pointer < m_recv_pointer; m_scan_pointer++, m_data_length--)
                        {
                            m_packet_data[m_scan_pointer - 16] = m_byte_buffer[m_scan_pointer];
                        }
                    }
                    if (m_data_length == 0)
                    {
                        return OVER;
                    }
                    else
                    {
                        return CONTINUE;
                    }
                }
            }
            else
            {
                return OVER;
            }
        }
        else
        {
            return ERROR;
        }
    }
    return CONTINUE;
}

void DataPacket::get_header()
{
    if ((m_recv_pointer >= 4) && (m_scan_pointer == 0)) {
        m_command = bytes_to_int(m_byte_buffer, 0);
        m_scan_pointer = 4;
    }
    if ((m_recv_pointer >= 8) && (m_scan_pointer == 4)) {
        m_data_length = bytes_to_int(m_byte_buffer, 4);
        m_packet_data.resize(m_data_length);
        m_scan_pointer = 8;
    }
    if ((m_recv_pointer >= 12) && (m_scan_pointer == 8)) {
        m_data_check = bytes_to_int(m_byte_buffer, 8);
        m_scan_pointer = 12;
    }
    if ((m_recv_pointer >= 16) && (m_scan_pointer == 12)) {
        m_magic = bytes_to_int(m_byte_buffer, 12);
        m_scan_pointer = 16;
    }
}

QByteArray DataPacket::int_to_bytes(unsigned int number, QByteArray byte_array, int offset)
{
    byte_array[offset]     = (uchar)  (0x000000FF & number);
    byte_array[offset + 1] = (uchar) ((0x0000FF00 & number) >> 8);
    byte_array[offset + 2] = (uchar) ((0x00FF0000 & number) >> 16);
    byte_array[offset + 3] = (uchar) ((0xFF000000 & number) >> 24);

    return byte_array;
}

unsigned int DataPacket::bytes_to_int(QByteArray bytearray, int offset)
{
    unsigned int number;

    number  =   bytearray[offset] & 0x000000FF;
    number |= ((bytearray[offset + 1] << 8)  & 0x0000FF00);
    number |= ((bytearray[offset + 2] << 16) & 0x00FF0000);
    number |= ((bytearray[offset + 3] << 24) & 0xFF000000);

    return number;
}
