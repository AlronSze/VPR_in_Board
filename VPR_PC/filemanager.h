#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QMainWindow>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "datapacket.h"

namespace Ui {
class FileManager;
}

class FileManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit FileManager(QWidget *parent = 0, QSerialPort *serialport_temp = NULL);
    ~FileManager();

private slots:
    void on_pushButton_select_clicked();
    void on_pushButton_storage_clicked();
    void on_pushButton_update_clicked();
    void on_pushButton_get_clicked();

    void delete_packet();
    void read_port_data();

    void on_pushButton_delete_clicked();

private:
    Ui::FileManager *ui;

    QSerialPort *serialport;
    DataPacket  *datapacket_r;
    QTimer      *timer;
    QString     m_file_name;
    bool        m_name_flag;
    bool        m_file_flag;

    void write_port_data(QByteArray data);
    void send_file();
    void get_file();
};

#endif // FILEMANAGER_H
