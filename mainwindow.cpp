#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;
#define WM_USER 0x0400
#define PBM_SETPOS (WM_USER+2)
#define MAX_BUFFER_SIZE 524288
#define HID_WRITE_LENGTH 65

bool SysStatus = false;
unsigned int fwsize = 0;
hid_device* usb_handle;
HWND pbar;
char fwbuffer[524288];

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    hid_init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//链接USB设备
int MainWindow::DevConnect()
{
    hid_device *handle = NULL;
    int SeekCount = 10;

    while (handle == NULL)
    {
        handle = hid_open(0x0416, 0x1200, NULL);

        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1100, NULL);

        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1101, NULL);
        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1102, NULL);
        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1103, NULL);
        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1104, NULL);
        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1105, NULL);
        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1106, NULL);
        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1107, NULL);
        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1108, NULL);
        if (handle != NULL) { break; }

        handle = hid_open(0x0416, 0x1109, NULL);
        if (handle != NULL) { break; }

        if(SeekCount < 1) { break; }

        SeekCount --;
        QThread::msleep(50);
    }

    if(handle == NULL)
    {
        ui->plainTextEdit->appendPlainText("ERROR! Compatible device not found.2");
        qDebug()<< SeekCount<<"handle == null";
        return 1;
    }else {
        ui->plainTextEdit->appendPlainText("Device connected.");
        qDebug()<< SeekCount<< "handle != null";
        usb_handle = handle;
        return 0;
    }
}

int MainWindow::usb_read(unsigned char *readbuf)
{
    int res;

    if (usb_handle == NULL) {
        ui->plainTextEdit->appendPlainText("Invalid USB handle");
        return -2;
    }

    memset(readbuf, 0, 64);
    res = hid_read(usb_handle, readbuf, 64);
    return res;
}

int MainWindow::write_page(int pageid)
{
    unsigned char buf[5];
    unsigned char buff[100] = {0};

    int res;
    buf[0] = 0x01;
    buf[1] = 0x83;
    buf[2] = 0x8C;
    buf[3] = 0x00;
    buf[4] = 0x00;

    if (pageid > 0xFF) {

        buf[3] = pageid - 256;
        buf[4] = int(pageid / 256);
    }
    else {
        buf[3] = pageid;
    }

    usb_send(buf);
    res = usb_read(buff);
    if(res < 0)
    {
        ui->plainTextEdit->appendPlainText("usb读取不成功1");
        qDebug()<<"usb读取不成功1";
    }

    if (buff[2] == 0x81) {
        ui->plainTextEdit->appendPlainText("ERROR: writing page failed.");
        return 1;
    }
    return 0;
}

int MainWindow::usb_send(unsigned char *buf)
{
    static int res;

    if (usb_handle == NULL) {
        ui->plainTextEdit->appendPlainText("Invalid USB handle");
        return -1;
    }
    res = hid_write(usb_handle, buf, 64);
    return res;
}

int MainWindow::send_page(unsigned int idx)
{
    int chks = 0;
    unsigned char buf[35] = {0};
    for (int i = 0; i < 4; i++)
    {
        buf[0] = 0x01;
        buf[1] = 0xA1;
        buf[2] = 0x8B;
        for (int j = 0; j < 32; j++) {
            if (idx < fwsize) {
                buf[j + 3] = fwbuffer[idx];
                idx++;
            }
            else {
                buf[j + 3] = 0;
            }
        }
        chks = chks + sendpkt(buf);
    }
    if (chks > 0)
    {
        return 0;
    }
    else if (chks < 0)
    {
        ui->plainTextEdit->appendPlainText(QString::asprintf("%d",chks));
        ui->plainTextEdit->appendPlainText("return -1");
        return -1;
    }
    else {
        return idx;
    }
}

int MainWindow::sendpkt(unsigned char *pktbuf)
{
    unsigned char tmpbuff[100] = {0};
    int tmp = 0;
    int chksum = 0;
    int res = 0;
    for (int i = 0; i < 16; i++) {
        tmp = (pktbuf[3 + (i * 2)] << 8) + pktbuf[3 + (i * 2 + 1)];
        chksum += tmp;
    }

    if (chksum > 0xffff)
    {
        chksum &= 0xffff;
    }

    static int count = 0;
    count ++;
    res = usb_send(pktbuf);
    if (res < 0)
    {
        ui->plainTextEdit->appendPlainText("sendpkt return -1" + QString::asprintf("count:%d",count));
        return  -1;
    }

    res = usb_read(tmpbuff);
    if(res < 0)
    {
        ui->plainTextEdit->appendPlainText("usb读取不成功2");
        qDebug()<<"usb读取不成功2";
    }

    tmp = (tmpbuff[4] << 8) + tmpbuff[3];
    if ((tmpbuff[2] == 0x80) && (tmp == chksum))
    {
//        ui->plainTextEdit->appendPlainText("Checksum message " + QString::asprintf("chksum:%d",chksum) + "; received " + QString::asprintf("tmp:%d",tmp));
        return 0;
    }
    else {
        ui->plainTextEdit->appendPlainText("Checksum error! Expected " + QString::asprintf("chksum:%d",chksum) + "; received " + QString::asprintf("tmp:%d",tmp));
        return 1;
    }
}

void MainWindow::on_actionAbout_Updata_triggered()
{
    QString Title = "About消息框";
    QString Information = "我开发的数据查看软件Updata V1.0.0";
    QMessageBox::about(this,Title,Information);
}

//读取烧写文件
void MainWindow::on_pBLoad_clicked()
{
    QSettings setting("./Setting.ini", QSettings::IniFormat);
    QString lastPath = setting.value("LastFilePath").toString();
    QString Title = "打开一个文件";
    QString filter = "程序文件(*.bin);;文本文件(*.txt)";
    QString filename = QFileDialog::getOpenFileName(this,Title,lastPath,filter);

    if(filename.isEmpty())
    {
        return;
    }
    else {
        setting.setValue("LastFilePath",filename);
    }

    QFile file(filename);
    if(!file.exists())
        return;
    if(!file.open(QIODevice::ReadOnly))
        return;
    ui->plainTextEdit->appendPlainText("Loading File:"+file.fileName());
    file.read(fwbuffer,MAX_BUFFER_SIZE);
    fwsize = file.size();
    ui->plainTextEdit->appendPlainText(QString::asprintf("Read bytes:%d",fwsize));
    file.close();
    return;
}

void free_hid_device(hid_device* usb_handle)
{
    hid_close(usb_handle);
    hid_exit();
}

int eof_cleanup_err()
{
    SysStatus = false;
    if (usb_handle != NULL) {
        free_hid_device(usb_handle);
    }
    return 1;
}

int eof_cleanup()
{
    SysStatus = false;
    if (usb_handle != NULL) {
        free_hid_device(usb_handle);
    }
    return 0;
}

//文件烧录过程
void MainWindow::on_pBRun_clicked()
{
    int res;
    wchar_t wstr[255];
    unsigned char sz = 0x00;
    unsigned char ret[100] = {0};
    unsigned int previdx = 0;
    unsigned int idx = 0;
    unsigned int pageid = 0;

    if(SysStatus)
        return;
    SysStatus = true;

    if(fwsize < 1)
    {
        ui->plainTextEdit->appendPlainText("ERROR! Load a binary first.");
        qDebug()<<"ERROR! Load a binary first.";
        eof_cleanup_err();
        return;
    }

    if(DevConnect() == 0)
    {
        res = hid_get_product_string(usb_handle, wstr,sizeof(wchar_t) * 255);
        if(res)
        {
            hid_error(usb_handle);
            return;
        }
        wstring ws = (wstring)wstr;
        string ss(ws.begin(),ws.end());
        ui->plainTextEdit->appendPlainText("Product String: "+ QString::fromStdString(ss));

        unsigned long i=300;
        while (ss.compare("BL") != 0)
        {
            if(i > 1000)
            {
                ui->plainTextEdit->appendPlainText("ERROR! Bootloader not found.1");
                eof_cleanup_err();
                return;
            }
            ui->plainTextEdit->appendPlainText("Initiating reset to enter BL mode...");

            res = usb_send1({ 0x81,0x83,0x00});//app进入BL指令
            QThread::msleep(500);
//            hid_close(usb_handle);
            eof_cleanup_err();
            ui->plainTextEdit->appendPlainText(QString::asprintf("res1=%d",res));

            if(res > 0 && DevConnect() == 0)
            {
                hid_get_product_string(usb_handle,wstr,sizeof(wchar_t)*255);
                ws = (wstring)wstr;
                ss = string(ws.begin(),ws.end());
                ui->plainTextEdit->appendPlainText("Product String after reset1: "+QString::fromStdString(ss));
                ui->plainTextEdit->appendPlainText("已进入Bootloader模式");

                res = usb_send1({ 0x81,0x83,0x00 });//保持在BL模式中指令
                QThread::msleep(500);

//                hid_close(usb_handle);
                eof_cleanup_err();
                if(res > 0 && DevConnect() == 0)
                {
                    ui->plainTextEdit->appendPlainText("已保持在Bootloader模式");
                }
                else {
                    ui->plainTextEdit->appendPlainText("未保持在Bootloader模式");
                    hid_exit();
                }
            }
            else {
                ui->plainTextEdit->appendPlainText("未进入Bootloader");
                hid_exit();
            }
            i += 100;
        }
    }
    else
    {
        ui->plainTextEdit->appendPlainText("ERROR! Connection error.");
        eof_cleanup_err();
        return;
    }

    if (fwsize > (128 * 1024)) {
        sz = 0x05;
    }
    else if (fwsize > (64 * 1024)) {
        sz = 0x04;
    }
    else if (fwsize > (32 * 1024)) {
        sz = 0x03;
    }
    else if (fwsize > (16 * 1024)) {
        sz = 0x02;
    }
    else if (fwsize > (8 * 1024)) {
        sz = 0x01;
    }

    ui->plainTextEdit->appendPlainText("Clearing flash...");
    usb_send1({ (unsigned char)0x8A, sz });

    res = usb_read(ret);
    if(res < 0)
    {
        ui->plainTextEdit->appendPlainText("usb读取不成功3");
        qDebug()<<"usb读取不成功3";
    }
    if ((ret[1] != 0x81) || (ret[2] != 0x80)) {
        ui->plainTextEdit->appendPlainText("ERROR: Flash clear failed!");
        eof_cleanup_err();
        return;
    }
    else {
        ui->plainTextEdit->appendPlainText("Clearing succeed");
    }

    ui->plainTextEdit->appendPlainText("Transfer started...");
    while (1) {
        previdx = idx;
        res = 0;
        while (res < 1) {
            res = send_page(previdx);
            if (res < 0) {
                ui->plainTextEdit->appendPlainText("ERROR: when transmitting,the device disconnected.");
                eof_cleanup_err();
                return;
            }
        }

        idx = res;
        write_page(pageid);
        ui->progressBar->setValue((100 * idx) / fwsize);
        pageid++;
        if (idx >= fwsize)
        {
            break;
        }
    }

    ui->plainTextEdit->appendPlainText("Transfer completed! Resetting back to AP.");
    res = usb_send1({ 0x81, 0x80, 0x00});
    QThread::msleep(1000);

    if (res > 0 && DevConnect() == 0) {
        res = hid_get_product_string(usb_handle, wstr,sizeof(wchar_t) * 255);
        wstring ws = (wstring)wstr;
        string ss(ws.begin(), ws.end());
        ui->plainTextEdit->appendPlainText("Product String: " + QString::fromStdString(ss));
        ui->plainTextEdit->appendPlainText("Done.");
        ui->plainTextEdit->appendPlainText("/----------------------------------------------/");
    }
    else {
        ui->plainTextEdit->appendPlainText("WARNING! Unable to verify device is working.");
        eof_cleanup_err();
        return;
    }
    eof_cleanup();
    return;
}

template<class T>
T MainWindow::usb_send1(std::initializer_list<T> list)
{
    static unsigned char buf[512];
    int res = 0;
    int i = 2;

    if (usb_handle == NULL) {
        ui->plainTextEdit->appendPlainText("Invalid USB handle");
        return res;
    }

    memset(buf, 0, 64);

    buf[0] = 0x01;
    buf[1] = 0x80 + list.size();
    for (auto elem : list)
    {
        buf[i++] = elem;
    }
//    res = hid_write(usb_handle, buf, HID_WRITE_LENGTH);
    res = hid_write(usb_handle, buf, 64);
    if (res < 1)
    {
        ui->plainTextEdit->appendPlainText(QString::asprintf("USB write error,res=%d",res));
    }
    return res;
}
