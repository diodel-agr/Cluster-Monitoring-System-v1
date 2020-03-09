#pragma once

#ifndef ENTERCREDENTIALSDIALOG_H
#define ENTERCREDENTIALSDIALOG_H

#include <QDialog>

#include "clientlistwindow.h"
#include "client.h"
#include "mainwindow.h"

namespace Ui {
    class ControlPanel;
}

class ControlPanel : public QDialog
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    explicit ControlPanel(QWidget *parent = nullptr, ClientTCP* client = nullptr);
    ~ControlPanel();

private slots:

    //void on_okButton_clicked();

    //void on_cancelButton_clicked();

    void on_button_logOut_clicked();

    void on_button_exit_clicked();

    void on_button_getClients_clicked();

signals:
    //void gotCredentials(QString email, QString password);

private:
    Ui::ControlPanel *ui;
    ClientTCP* client;
};

#endif // CONTROLPANEL_H
