#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QQuickWidget>
#include <QtCore/QtCore>
#include <QTimer>
#include "networkmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private slots:
    void apRefresh(QList<QMap<QString, QString> > ap);
    void apRemove(QMap<QString, QString> ap); /*Unused - just checking how it works*/
    void apAdd(QMap<QString, QString> ap); /*Unused - just checking how it works*/
    void timerTimeOut();
    void cancelScan();

    void on_refreshBtn_clicked();

    void on_updateActivation_stateChanged(int arg1);

  private:
    Ui::MainWindow *ui;
    NetworkManager *nm;
    QTimer timer;

    const int COLUMN_SSID = 0;
    const int COLUMN_MAC = 1;
    const int COLUMN_STR = 2;
    const int COLUMN_FREQ = 3;
    const int COLUMN_PATH = 4;

    QList<int> getApIndexByPath(const QMap<QString, QString> &ap); /*Unused - just checking how it works*/
    void insertApToTable(const QMap<QString, QString> &ap); /*Unused - just checking how it works*/

    QString getIconWifiStr(int apStr);

};
#endif // MAINWINDOW_H
