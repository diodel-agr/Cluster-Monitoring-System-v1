#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void slotGotCredentials(QString email, QString password);

    void on_button_exit_clicked();

    void on_button_login_clicked();

private:
    Ui::MainWindow *ui;
    bool apasa = false;
};

#endif // MAINWINDOW_H
