#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "hidapi.h"
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QThread>
#include <errhandlingapi.h>
#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    int DevConnect();
    template <class T> T usb_send1(std::initializer_list<T> list);
//    unsigned char* usb_read();
    int usb_read(unsigned char * buf);
    int write_page(int pageid);
    int usb_send(unsigned char* buf);
    int send_page(unsigned int idx);
    int sendpkt(unsigned char* buf);

private slots:
    void on_actionAbout_Updata_triggered();

    void on_pBLoad_clicked();

    void on_pBRun_clicked();

private:
    Ui::MainWindow *ui;
    hid_device *handle;
};

#endif // MAINWINDOW_H
