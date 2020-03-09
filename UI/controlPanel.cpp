#include "controlPanel.h"
#include "ui_controlPanel.h"

ControlPanel::ControlPanel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ControlPanel)
{
    ui->setupUi(this);
}

ControlPanel::ControlPanel(QWidget *parent, ClientTCP* client) :
    QDialog(parent),
    ui(new Ui::ControlPanel)
{
    ui->setupUi(this);
    this->client = client;
}

ControlPanel::~ControlPanel()
{
    delete ui;
}

//void ControlPanel::on_okButton_clicked()
//{
//    //emit gotCredentials(mEmail, mPassword);
//    this->close();
//}

void ControlPanel::on_button_logOut_clicked()
{
    /* disconnect from server. */
    this->client->disconnectFromServer();
    delete this->client;
    /* open log-in window. */
    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();
    /* close this window. */
    this->close();
}

void ControlPanel::on_button_exit_clicked()
{
    /* disconnect from server. */
    this->client->disconnectFromServer();
    delete this->client;
    /* close this window */
    this->close();
}

void ControlPanel::on_button_getClients_clicked()
{
    /* open next window. */
    ClientListWindow* clientListWindow = new ClientListWindow(nullptr, this->client);
    clientListWindow->show();
    /* close this window. */
    this->close();
}
