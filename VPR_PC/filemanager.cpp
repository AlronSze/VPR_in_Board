#include "filemanager.h"
#include "ui_filemanager.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDebug>

FileManager::FileManager(QWidget *parent, QSerialPort *serialport_temp) :
    QMainWindow(parent),
    ui(new Ui::FileManager),
    m_name_flag(false),
    m_file_flag(false)
{
    serialport = serialport_temp;
    connect(serialport, SIGNAL(readyRead()), this, SLOT(read_port_data()));

    datapacket_r = new DataPacket();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(delete_packet()));

    ui->setupUi(this);
    on_pushButton_update_clicked();
}

FileManager::~FileManager()
{
    delete ui;
}

void FileManager::delete_packet()
{
    delete datapacket_r;
    datapacket_r = new DataPacket();
    timer->stop();
}

void FileManager::read_port_data()
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

    if ((datapacket_r->get_command() == SENDNAME) && m_name_flag)
    {
        m_name_flag = false;
        send_file();
    }
    else if ((datapacket_r->get_command() == SENDFILE) && m_file_flag)
    {
        m_file_flag = false;
        QMessageBox::information(this, tr("information"), tr("File saved!         "), QMessageBox::Ok);
        on_pushButton_update_clicked();
    }
    else if (datapacket_r->get_command() == LISTFILE)
    {
        QString one_file = datapacket_r->get_byte_array();
        ui->listWidget->addItem(one_file);

        DataPacket datapacket;
        write_port_data(datapacket.pack_data(LISTFILE, NULL));
    }
    else if (datapacket_r->get_command() == GETFILE)
    {
        get_file();
        QMessageBox::information(this, tr("information"), tr("File is got!         "), QMessageBox::Ok);
    }
    else if (datapacket_r->get_command() == DELFILE)
    {
        QMessageBox::information(this, tr("information"), tr("File is deleted!         "), QMessageBox::Ok);
        on_pushButton_update_clicked();
    }
    else
    {
        qDebug() << "No Match!";
    }

    delete_packet();
}

void FileManager::write_port_data(QByteArray data)
{
    serialport->write(data);
}

void FileManager::on_pushButton_select_clicked()
{
    QFileDialog *filedialog = new QFileDialog(this);
    filedialog->setWindowTitle(tr("Open File"));
    filedialog->setDirectory(".");
    if(filedialog->exec() == QDialog::Accepted)
    {
        QString path = filedialog->selectedFiles()[0];
        ui->lineEdit->setText(path);
        ui->pushButton_storage->setEnabled(true);
    }
    else
    {
        ui->pushButton_storage->setEnabled(false);
        QMessageBox::warning(NULL, tr("warning"), tr("You didn't select any files."));
    }
    delete filedialog;
}

void FileManager::on_pushButton_storage_clicked()
{
    m_name_flag = true;
    ui->lineEdit->setEnabled(false);

    QString str = ui->lineEdit->text();

    QFile file(str);
    QFileInfo fileinfo = QFileInfo(file);
    QString file_name = fileinfo.fileName();

    DataPacket datapacket;
    write_port_data(datapacket.pack_data(SENDNAME, file_name.toLatin1()));
}

void FileManager::send_file()
{
    m_file_flag = true;
    QString str = ui->lineEdit->text();

    QFile file(str);
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();
    file.close();

    DataPacket datapacket;
    write_port_data(datapacket.pack_data(SENDFILE, file_data));
}

void FileManager::get_file()
{
    QFile file(".\\file\\" + m_file_name);
    file.open(QIODevice::WriteOnly);
    file.write(datapacket_r->get_byte_array());
    file.close();

    m_file_name = "";
}

void FileManager::on_pushButton_update_clicked()
{
    ui->listWidget->clear();
    DataPacket datapacket;
    write_port_data(datapacket.pack_data(LISTFILE, NULL));
}

void FileManager::on_pushButton_get_clicked()
{
    if (ui->listWidget->currentItem() == NULL)
    {
        QMessageBox::warning(NULL, tr("warning"), tr("You didn't select any files."));
        return;
    }
    m_file_name = ui->listWidget->currentItem()->text();

    DataPacket datapacket;
    write_port_data(datapacket.pack_data(GETFILE, m_file_name.toLatin1()));
}

void FileManager::on_pushButton_delete_clicked()
{
    if (ui->listWidget->currentItem() == NULL)
    {
        QMessageBox::warning(NULL, tr("warning"), tr("You didn't select any files."));
        return;
    }
    m_file_name = ui->listWidget->currentItem()->text();

    DataPacket datapacket;
    write_port_data(datapacket.pack_data(DELFILE, m_file_name.toLatin1()));
}
