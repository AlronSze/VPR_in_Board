#include "loginmanager.h"
#include "ui_loginmanager.h"

#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <stdio.h>
#include <stdlib.h>

LoginManager::LoginManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginManager),
    m_file_index(0)
{
    recordvoice = new RecordVoice();
    init_port();
    recordvoice->init_record();
    datapacket_r = new DataPacket();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(delete_packet()));
    ui->setupUi(this);
}

LoginManager::~LoginManager()
{
    delete ui;
}

void LoginManager::init_port(void)
{
    serialport = new QSerialPort();
    serialport->setPortName("/dev/ttyS1"); // For PC debug
    // serialport->setPortName("/dev/ttySAC1"); // For board
    serialport->open(QIODevice::ReadWrite);
    connect(serialport, SIGNAL(readyRead()), this, SLOT(read_port_data()));
    serialport->setBaudRate(115200);
    serialport->setDataBits(QSerialPort::Data8);
    serialport->setParity(QSerialPort::NoParity);
    serialport->setStopBits(QSerialPort::OneStop);
    serialport->setFlowControl(QSerialPort::NoFlowControl);
}

void LoginManager::write_port_data(QByteArray data)
{
    serialport->write(data);
}

void LoginManager::delete_packet()
{
    delete datapacket_r;
    datapacket_r = new DataPacket();
    timer->stop();
}

void LoginManager::read_port_data(void)
{
    QByteArray read_buf = serialport->readAll();
    unsigned char recv_case = datapacket_r->unpack_data(read_buf);

    if (recv_case == CONTINUE)
    {
        timer->start(5000);
        return;
    }
    else if (recv_case == ERROR)
    {
        delete_packet();
        return;
    }

    timer->stop();
    DataPacket datapacket_s;

    if (datapacket_r->get_command() == RECORD)
    {
        start_record();
        write_port_data(datapacket_s.pack_data(RECORD, NULL));
    }
    else if (datapacket_r->get_command() == REPLAY)
    {
        start_replay();
        write_port_data(datapacket_s.pack_data(REPLAY, NULL));
    }
    else if (datapacket_r->get_command() == LOGIN)
    {
        QByteArray reply("O");
        QByteArray reply_error("E");
        if (start_login())
        {
            write_port_data(datapacket_s.pack_data(LOGIN, reply));
        }
        else
        {
            write_port_data(datapacket_s.pack_data(LOGIN, reply_error));
        }
    }
    else if (datapacket_r->get_command() == REGISTER)
    {
        QByteArray reply("O");
        QByteArray reply_error("E");
        if (start_reg())
        {
            write_port_data(datapacket_s.pack_data(REGISTER, reply));
        }
        else
        {
            write_port_data(datapacket_s.pack_data(REGISTER, reply_error));
        }
    }
    else if (datapacket_r->get_command() == SENDNAME)
    {
        create_file();
        write_port_data(datapacket_s.pack_data(SENDNAME, NULL));
    }
    else if (datapacket_r->get_command() == SENDFILE)
    {
        storage_file();
        write_port_data(datapacket_s.pack_data(SENDFILE, NULL));
    }
    else if (datapacket_r->get_command() == LISTFILE)
    {
        QByteArray file_name = send_file_name();
        if (!file_name.isEmpty())
        {
            write_port_data(datapacket_s.pack_data(LISTFILE, file_name));
        }
    }
    else if (datapacket_r->get_command() == GETFILE)
    {
        send_file_data(datapacket_s);
    }
    else if (datapacket_r->get_command() == DELFILE)
    {
        delete_file_data();
        write_port_data(datapacket_s.pack_data(DELFILE, NULL));
    }
    else
    {
        qDebug() << "No Match!";
    }

    delete_packet();
}

bool LoginManager::start_login()
{
    QFile file("/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file/aaa"); // For PC debug
    // QFile file("/VPR/file/aaa"); // For board
    if(!file.exists())
    {
        QMessageBox *messageBox = new QMessageBox(QMessageBox::Warning, tr("Warning"), tr("No register!"), NULL, this);
        messageBox->show();
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
        timer->start(2000);
        return false;
    }

    file.open(QIODevice::ReadOnly);
    QByteArray info = file.readAll();
    file.close();

    if (info == datapacket_r->get_byte_array())
    {
        QMessageBox *messageBox = new QMessageBox(QMessageBox::Information, tr("Information"), tr("Login successful!"), NULL, this);
        messageBox->show();
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
        timer->start(2000);
        return true;
    }
    else
    {
        QMessageBox *messageBox = new QMessageBox(QMessageBox::Warning, tr("Warning"), tr("Login Error!"), NULL, this);
        messageBox->show();
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
        timer->start(2000);
        return false;
    }
}

bool LoginManager::start_reg()
{
    QDir dir("/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file"); // For PC debug
    // QDir dir("/VPR/file"); // For board
    if (!dir.exists())
    {
        QDir dir2("/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release"); // For PC debug
        // QDir dir2("/VPR"); // For board
        dir2.mkdir("file");
    }

    QFile file("/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file/aaa"); // For PC debug
    // QFile file("/VPR/file/aaa"); // For board
    if(file.exists())
    {
        QMessageBox *messageBox = new QMessageBox(QMessageBox::Warning, tr("Warning"), tr("Already register!"), NULL, this);
        messageBox->show();
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
        timer->start(2000);
        return false;
    }

    delete_dir_file();

    file.open(QIODevice::WriteOnly);
    file.write(datapacket_r->get_byte_array());
    file.close();

    /* For board
    MFCC mfcc;
    mfcc.StartMFCC();
    */

    QMessageBox *messageBox = new QMessageBox(QMessageBox::Information, tr("Information"), tr("Register successful!"), NULL, this);
    messageBox->show();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
    timer->start(2000);
    return true;
}

void LoginManager::start_record()
{
    recordvoice->start_record();
    QMessageBox *messageBox = new QMessageBox(QMessageBox::Information, tr("Information"), tr("Your record is over!"), NULL, this);
    messageBox->show();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
    timer->start(2000);
}

void LoginManager::start_replay()
{
    recordvoice->play_record();
    QMessageBox *messageBox = new QMessageBox(QMessageBox::Information, tr("Information"), tr("Your replay is over!"), NULL, this);
    messageBox->show();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
    timer->start(2000);
}

void LoginManager::create_file()
{
    m_file_name = "/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file/data/" + datapacket_r->get_byte_array(); // For PC debug
    // m_file_name = "/VPR/file/data/" + datapacket_r->get_byte_array(); // For board
    QFile file(m_file_name);
    file.open(QIODevice::WriteOnly);
    file.close();
}

void LoginManager::storage_file()
{
    QFile file(m_file_name);
    file.open(QIODevice::WriteOnly);
    file.write(datapacket_r->get_byte_array());
    file.close();

    QMessageBox *messageBox = new QMessageBox(QMessageBox::Information, tr("Information"), tr("Your file is saved!"), NULL, this);
    messageBox->show();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
    timer->start(2000);
}

void LoginManager::send_file_data(DataPacket datapacket)
{
    QString file_name = "/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file/data/" + datapacket_r->get_byte_array(); // For PC debug
    // QString file_name = "/VPR/file/data/" + datapacket_r->get_byte_array(); // For board

    QFile file(file_name);
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();
    file.close();

    write_port_data(datapacket.pack_data(GETFILE, file_data));

    QMessageBox *messageBox = new QMessageBox(QMessageBox::Information, tr("Information"), tr("Your file is sent!"), NULL, this);
    messageBox->show();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
    timer->start(2000);
}

void LoginManager::delete_file_data()
{
    QDir dir("/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file/data"); // For PC debug
    // QDir dir("/VPR/file/data"); // For board

    dir.setFilter(QDir::Files);
    for (int i = 0, j = dir.count() - 1; i <= j; i++)
    {
        QFileInfoList fileinfolist = dir.entryInfoList();
        QFileInfo file_info = fileinfolist.at(i);
        QByteArray file_name = file_info.fileName().toLatin1();
        if (file_name == datapacket_r->get_byte_array())
        {
            dir.remove(dir[i]);
        }
    }

    QMessageBox *messageBox = new QMessageBox(QMessageBox::Information, tr("Information"), tr("Your file is deleted!"), NULL, this);
    messageBox->show();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), messageBox, SLOT(close()));
    timer->start(2000);
}

QByteArray LoginManager::send_file_name()
{
    QDir dir("/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file/data"); // For PC debug
    // QDir dir("/VPR/file/data"); // For board
    if (!dir.exists())
    {
        QDir dir2("/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file"); // For PC debug
        // QDir dir2("/VPR/file"); // For board
        dir2.mkdir("data");
    }

    dir.setFilter(QDir::Files);
    if (m_file_index >= dir.count())
    {
        m_file_index = 0;
        return NULL;
    }
    else
    {
        QFileInfoList fileinfolist = dir.entryInfoList();
        QFileInfo file_info = fileinfolist.at(m_file_index);
        QByteArray file_name = file_info.fileName().toLatin1();
        m_file_index++;
        return file_name;
    }
}

void LoginManager::delete_dir_file()
{
    QDir dir("/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file/data"); // For PC debug
    // QDir dir("/VPR/file/data"); // For board
    if (!dir.exists())
    {
        QDir dir2("/home/al/Desktop/VPR_Board-build-desktop-Qt_4_8_5__x11__Release/file"); // For PC debug
        // QDir dir2("/VPR/file"); // For board
        dir2.mkdir("data");
        return;
    }

    dir.setFilter(QDir::Files);
    for (int i = 0, j = dir.count() - 1; i <= j; i++)
    {
        dir.remove(dir[i]);
    }
}
