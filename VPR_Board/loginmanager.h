#ifndef LOGINMANAGER_H
#define LOGINMANAGER_H

#include "recordvoice.h"
#include "datapacket.h"
#include "mfcc.h"
#include "rsa.h"

#include <QMainWindow>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class LoginManager;
}

class LoginManager : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginManager(QWidget *parent = 0);
    ~LoginManager();

private slots:
    void read_port_data(void);
    void delete_packet(void);

private:
    Ui::LoginManager *ui;

    RecordVoice *recordvoice;
    QSerialPort *serialport;
    QTimer      *timer;
    DataPacket  *datapacket_r;
    RSA         *rsa;
    QString      m_file_name;
    unsigned int m_file_index;
    bool         m_file_begin;
    bool         m_rsa_e_n_flag;

    void init_port(void);
    void write_port_data(QByteArray data);

    void start_record(void);
    void start_replay(void);
    bool start_login(void);
    bool start_reg(void);
    void create_file(void);
    void storage_file(void);
    QByteArray send_file_name(void);
    void send_file_data(DataPacket datapacket);
    void delete_file_data(void);
    void delete_dir_file(void);
};

#endif // LOGINMANAGER_H
