#include "clientlistwindow.h"
#include "ui_clientlistwindow.h"

ClientListWindow::ClientListWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientListWindow)
{
    ui->setupUi(this);
}

ClientListWindow::ClientListWindow(QWidget *parent, ClientTCP* client) :
    QWidget(parent),
    ui(new Ui::ClientListWindow)
{
    ui->setupUi(this);
    this->client = client;
    this->on_button_refresh_clicked();
}

ClientListWindow::~ClientListWindow()
{
    delete ui;
}

void ClientListWindow::on_button_exit_clicked()
{
    /* disconnect from server. */
    this->client->disconnectFromServer();
    delete this->client;
    this->client = nullptr;
    /* close this window. */
    this->close();
}

void ClientListWindow::on_button_back_clicked()
{
    ControlPanel* ctrlPanel = new ControlPanel(nullptr, this->client);
    ctrlPanel->show();
    /* close this window. */
    this->close();
}

void ClientListWindow::on_button_refresh_clicked()
{
    char* buffer = new char[BUFFER_SIZE];
    int len = this->client->getClientList(buffer, BUFFER_SIZE);
    /* write list view. */
    QStringListModel* model = new QStringListModel(this);
    QStringList list;
    char* name = strtok(buffer + 2, ":");
    while (name != nullptr)
    {
        QString text = name;
        list << text;
        name = strtok(NULL, ":");
    }
    model->setStringList(list);
    ui->listView->setModel(model);
    delete[] buffer;
}
