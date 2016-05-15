#ifndef LOGINMANAGER_H
#define LOGINMANAGER_H

#include "filemanager.h"
#include "datapacket.h"
#include "sm4.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#define SALT     "salt"
#define MATCH    "^[a-zA-Z_][a-zA-Z_0-9]*"
#define INFOMIN  6
#define INFOMAX  24

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
    void on_pushButton_login_clicked();
    void on_pushButton_reg_clicked();
    void on_pushButton_exit_clicked();
    void on_pushButton_record_clicked();
    void on_pushButton_replay_clicked();
    void on_checkBox_showPwd_clicked();
    void on_pushButton_update_clicked();
    void on_pushButton_open_clicked();

    void read_port_data();
    void delete_packet();
    void board_no_responce();

private:
    Ui::LoginManager *ui;

    FileManager *filemanager;
    QSerialPort *serialport;
    DataPacket  *datapacket_r;
    QTimer      *timer, *timer2;
    QByteArray  m_key;

    bool m_open_flag;
    bool m_record_flag;
    bool m_replay_flag;
    bool m_login_flag;
    bool m_reg_flag;
    bool m_rsa_key_flag;
    bool m_is_record;

    void write_port_data(QByteArray data);
    void start_login(void);
    void start_register(void);
    bool match_info(QString acct_str, QString pwd_str);
    QString get_md5(QString origin_str, QString md5_salt = SALT);
};

#endif // LOGINMANAGER_H
