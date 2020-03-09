#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "controlPanel.h"
#include "client.h"
#include "adapt.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->spinBox->setRange(0,360);
    ui->horizontalSlider->setRange(0,360);
    connect(ui->spinBox,SIGNAL(valueChanged(int)),ui->horizontalSlider,SLOT(setValue(int)));
    connect(ui->horizontalSlider,SIGNAL(valueChanged(int)),ui->spinBox,SLOT(setValue(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotGotCredentials(QString email, QString password)
{
    qInfo() << "Email: " << email;
    qInfo() << "Password: " << password;
}

void MainWindow::on_button_exit_clicked()
{
    this->close();
}

void MainWindow::on_button_login_clicked()
{
    qInfo() << "Login button clicked.\n";
    qInfo() << "User name: " << ui->lineEdit_username->text() << ", password: " << ui->lineEdit_password->text() << ".\n";
    /* create client and authenticate. */
    QString name = ui->lineEdit_username->text();
    QString pass = ui->lineEdit_password->text();
    ClientTCP* client = new ClientTCP(name, pass, DEFAULT_ADDRESS, DEFAULT_PORT);
    client->init();
    /* connect to server. */
    client->connectToServer();
    /* if authentication is successfull, go to control panel window, show error message otherwise. */
    bool connResult = client->authenticate();
    if (connResult == true)
    {
        /* open control panel. */
        qInfo() << "Authentication success.\n";
        ControlPanel *credDialog = new ControlPanel(nullptr, client);
        //connect(credDialog,SIGNAL(gotCredentials(QString, QString)),this,SLOT(slotGotCredentials(QString, QString)));
        credDialog->show();
        /* close this window. */
        this->close();
    }
    else
    {
        qInfo() << "Authentication failure!\n";
        client->disconnectFromServer();
        delete client;
    }
}
