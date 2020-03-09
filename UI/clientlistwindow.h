#pragma once

#ifndef CLIENTLISTWINDOW_H
#define CLIENTLISTWINDOW_H

#include <QWidget>
#include <QStringListModel>

#include "client.h"
#include "controlPanel.h"

namespace Ui {
class ClientListWindow;
}

class ClientListWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ClientListWindow(QWidget *parent = nullptr);
    explicit ClientListWindow(QWidget *parent = nullptr, ClientTCP* client = nullptr);
    ~ClientListWindow();

private slots:
    void on_button_exit_clicked();

    void on_button_back_clicked();

    void on_button_refresh_clicked();

private:
    Ui::ClientListWindow *ui;
    ClientTCP* client;
};

#endif // CLIENTLISTWINDOW_H
