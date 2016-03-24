#include "loginmanager.h"
#include "ui_loginmanager.h"

#include <QDebug>
#include <QCryptographicHash>

LoginManager::LoginManager(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginManager),
    m_record_flag(false),
    m_replay_flag(false),
    m_login_flag(false),
    m_reg_flag(false)
{
    ui->setupUi(this);
    on_pushButton_update_clicked();
}

LoginManager::~LoginManager()
{
    delete ui;
}

void LoginManager::delete_packet()
{
    delete datapacket_r;
    datapacket_r = new DataPacket();
    timer->stop();
}

void LoginManager::read_port_data()
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

    if ((datapacket_r->get_command() == RECORD) && m_record_flag)
    {
        m_record_flag = false;
        ui->pushButton_record->setText("Record Start");
        ui->pushButton_record->setEnabled(true);
        ui->pushButton_login ->setEnabled(true);
        ui->pushButton_reg   ->setEnabled(true);
        ui->pushButton_replay->setEnabled(true);
        ui->pushButton_exit->setEnabled(true);
        QMessageBox::information(this, tr("information"), tr("Record over!         "), QMessageBox::Ok);
    }
    else if ((datapacket_r->get_command() == REPLAY) && m_replay_flag)
    {
        m_replay_flag = false;
        ui->pushButton_replay->setText("Record Replay");
        ui->pushButton_record->setEnabled(true);
        ui->pushButton_login ->setEnabled(true);
        ui->pushButton_reg   ->setEnabled(true);
        ui->pushButton_replay->setEnabled(true);
        ui->pushButton_exit->setEnabled(true);
        QMessageBox::information(this, tr("information"), tr("Replay over!         "), QMessageBox::Ok);
    }
    else if ((datapacket_r->get_command() == LOGIN) && m_login_flag)
    {
        m_login_flag = false;
        ui->pushButton_login->setText("Login");
        ui->pushButton_record->setEnabled(true);
        ui->pushButton_login ->setEnabled(true);
        ui->pushButton_reg   ->setEnabled(true);
        ui->pushButton_replay->setEnabled(true);
        ui->pushButton_exit->setEnabled(true);

        QByteArray judge("O");
        if (datapacket_r->get_byte_array() == judge)
        {
            QMessageBox::information(this, tr("information"), tr("Login over!         "), QMessageBox::Ok);
            serialport->disconnect();
            FileManager *filemanager = new FileManager(0, serialport);
            filemanager->show();
            this->close();
        }
        else
        {
            QMessageBox::warning(this, tr("warning"), tr("Login Error!         "), QMessageBox::Ok);
        }
    }
    else if ((datapacket_r->get_command() == REGISTER) && m_reg_flag)
    {
        m_reg_flag = false;
        ui->pushButton_reg->setText("Register");
        ui->pushButton_record->setEnabled(true);
        ui->pushButton_login ->setEnabled(true);
        ui->pushButton_reg   ->setEnabled(true);
        ui->pushButton_replay->setEnabled(true);
        ui->pushButton_exit->setEnabled(true);

        QByteArray judge("O");
        if (datapacket_r->get_byte_array() == judge)
        {
            QMessageBox::information(this, tr("information"), tr("Register over!         "), QMessageBox::Ok);
        }
        else
        {
            QMessageBox::warning(this, tr("warning"), tr("Already register!         "), QMessageBox::Ok);
        }
    }
    else
    {
        qDebug() << "No Match!";
    }

    delete_packet();
}

void LoginManager::write_port_data(QByteArray data)
{
    serialport->write(data);
}

bool LoginManager::match_info(QString acct_str, QString pwd_str)
{
    int acct_len = acct_str.length();
    int psw_len  = pwd_str.length();

    if (acct_len >= INFOMIN && acct_len <= INFOMAX)
    {
        QString pattern(MATCH);
        QRegExp regexp(pattern);

        if (acct_str.indexOf(regexp) == 0)
        {
            if (psw_len >= INFOMIN && psw_len <= INFOMAX)
            {
                return true;
            }
        }
    }

    return false;
}

QString LoginManager::get_md5(QString origin_str, QString md5_salt)
{
    QString salt_str, md5_final;
    QByteArray bb;

    salt_str = origin_str + md5_salt;
    bb = QCryptographicHash::hash(salt_str.toLatin1(), QCryptographicHash::Md5);
    md5_final.append(bb.toHex());

    return md5_final;
}

void LoginManager::on_checkBox_showPwd_clicked()
{
    if(ui->checkBox_showPwd->isChecked())
    {
        ui->lineEdit_pwd->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        ui->lineEdit_pwd->setEchoMode(QLineEdit::Password);
    }
}

void LoginManager::on_pushButton_login_clicked()
{
    QString acct_str(ui->lineEdit_acct->text());
    QString pwd_str(ui->lineEdit_pwd->text());

    if (match_info(acct_str, pwd_str))
    {
        acct_str = get_md5(acct_str.toLower());
        pwd_str  = get_md5(pwd_str);

        QString final_str = get_md5(acct_str + pwd_str);
        QByteArray packet_data = final_str.toLatin1();

        ui->pushButton_record->setEnabled(false);
        ui->pushButton_login->setEnabled(false);
        ui->pushButton_reg->setEnabled(false);
        ui->pushButton_replay->setEnabled(false);
        ui->pushButton_exit->setEnabled(false);
        ui->pushButton_login->setText("Logining...");

        DataPacket datapacket;
        write_port_data(datapacket.pack_data(LOGIN, packet_data));
        m_login_flag = true;
    }
    else
    {
        QMessageBox::warning(this, tr("Warning!"), tr("Input error!"), QMessageBox::Ok);
    }
}

void LoginManager::on_pushButton_reg_clicked()
{
    QString acct_str(ui->lineEdit_acct->text());
    QString pwd_str(ui->lineEdit_pwd->text());

    if (match_info(acct_str, pwd_str))
    {
        acct_str = get_md5(acct_str.toLower());
        pwd_str  = get_md5(pwd_str);

        QString final_str = get_md5(acct_str + pwd_str);
        QByteArray packet_data = final_str.toLatin1();

        ui->pushButton_record->setEnabled(false);
        ui->pushButton_login->setEnabled(false);
        ui->pushButton_reg->setEnabled(false);
        ui->pushButton_replay->setEnabled(false);
        ui->pushButton_exit->setEnabled(false);
        ui->pushButton_reg->setText("Registering...");

        DataPacket datapacket;
        write_port_data(datapacket.pack_data(REGISTER, packet_data));
        m_reg_flag = true;
    }
    else
    {
        QMessageBox::warning(this, tr("Warning!"), tr("Input error!"), QMessageBox::Ok);
    }
}

void LoginManager::on_pushButton_exit_clicked()
{
    this->close();
}

void LoginManager::on_pushButton_record_clicked()
{

    ui->pushButton_record->setEnabled(false);
    ui->pushButton_login->setEnabled(false);
    ui->pushButton_reg->setEnabled(false);
    ui->pushButton_replay->setEnabled(false);
    ui->pushButton_exit->setEnabled(false);
    ui->pushButton_record->setText("Recording...");

    DataPacket datapacket;
    write_port_data(datapacket.pack_data(RECORD, NULL));
    m_record_flag = true;
}

void LoginManager::on_pushButton_replay_clicked()
{
    ui->pushButton_record->setEnabled(false);
    ui->pushButton_login->setEnabled(false);
    ui->pushButton_reg->setEnabled(false);
    ui->pushButton_replay->setEnabled(false);
    ui->pushButton_exit->setEnabled(false);
    ui->pushButton_replay->setText("Replaying...");

    DataPacket datapacket;
    write_port_data(datapacket.pack_data(REPLAY, NULL));
    m_replay_flag = true;
}

void LoginManager::on_pushButton_update_clicked()
{
    ui->comboBox_port->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if (serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox_port->addItem(info.portName());
            serial.close();
        }
    }
}

void LoginManager::on_pushButton_open_clicked()
{
    serialport = new QSerialPort();

    if(ui->comboBox_port->currentText() == tr(""))
    {
        QMessageBox::warning(this,tr("Warning!"),tr("Unknown device!\nTry again"),QMessageBox::Ok);
        return;
    }
    serialport->setPortName(ui->comboBox_port->currentText());
    serialport->open(QIODevice::ReadWrite);
    connect(serialport, SIGNAL(readyRead()), this, SLOT(read_port_data()));

    serialport->open(QIODevice::ReadWrite);
    serialport->setBaudRate(115200);
    serialport->setDataBits(QSerialPort::Data8);
    serialport->setParity(QSerialPort::NoParity);
    serialport->setStopBits(QSerialPort::OneStop);
    serialport->setFlowControl(QSerialPort::NoFlowControl);

    datapacket_r = new DataPacket();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(delete_packet()));

    ui->pushButton_update->setEnabled(false);
    ui->pushButton_open->setEnabled(false);
    ui->pushButton_record->setEnabled(true);
}
