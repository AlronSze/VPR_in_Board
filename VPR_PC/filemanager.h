#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QMainWindow>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <stdint.h>

#include "datapacket.h"
#include "sm4.h"

namespace Ui {
class FileManager;
}

class FileManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit FileManager(QWidget *parent = 0, QSerialPort *serialport_temp = NULL, QByteArray key = NULL);
    ~FileManager();

private slots:
    void on_pushButton_select_clicked();
    void on_pushButton_storage_clicked();
    void on_pushButton_update_clicked();
    void on_pushButton_get_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_exit_clicked();

    void delete_packet();
    void read_port_data();

private:
    Ui::FileManager *ui;

    QSerialPort *serialport;
    DataPacket  *datapacket_r;
    QTimer      *timer;
    QString      m_file_name;
    bool         m_name_flag;
    bool         m_file_flag;
    uint32_t     m_key[4];

    void write_port_data(QByteArray data);
    void send_file();
    void get_file();
};

#endif // FILEMANAGER_H
