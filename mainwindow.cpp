#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow) {
  ui->setupUi(this);

  ui->quickWidget->hide();

  ui->quickWidget->setAttribute(Qt::WA_AlwaysStackOnTop, true);
  ui->quickWidget->setAttribute(Qt::WA_TranslucentBackground, true);
  ui->quickWidget->setClearColor(Qt::transparent);

  nm = new NetworkManager(parent);
  timer.setSingleShot(true);
  timer.setInterval(10000);

  QObject::connect(nm, SIGNAL(apRefreshAll(QList<QMap<QString, QString> >)), this, SLOT(apRefresh(QList<QMap<QString, QString> >)));
  QObject::connect(nm, SIGNAL(apAdd(QMap<QString, QString>)), this, SLOT(apAdd(QMap<QString, QString>)));
  QObject::connect(nm, SIGNAL(apRemove(QMap<QString, QString>)), this, SLOT(apRemove(QMap<QString, QString>)));
  QObject::connect(nm, SIGNAL(scanError()), this, SLOT(cancelScan()));
  QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(timerTimeOut()));

  ui->apList->setColumnHidden(COLUMN_PATH, true);
  ui->statusBar->setStyleSheet("color: red");

  ui->apList->setColumnWidth(COLUMN_SSID, 190);
  ui->apList->setColumnWidth(COLUMN_MAC, 150);
  ui->apList->setColumnWidth(COLUMN_FREQ, 60);
  ui->apList->setColumnWidth(COLUMN_STR, 60);
}

MainWindow::~MainWindow() {
  delete ui;
  delete nm;
}

void MainWindow::apRefresh(QList<QMap<QString, QString> > ap) {
  ui->apList->setRowCount(0);

  for(const QMap<QString, QString> &map : ap) {
    insertApToTable(map);
  }

  ui->apList->sortByColumn(COLUMN_SSID, Qt::AscendingOrder);
  ui->quickWidget->close();

  if(ui->updateActivation->checkState() == Qt::Checked) {
    timer.start();
  }
}

void MainWindow::insertApToTable(const QMap<QString, QString> &ap) {
  int row = ui->apList->rowCount();
  ui->apList->setRowCount(row + 1);

  QTableWidgetItem *item = new QTableWidgetItem(ap.value("SSID"));
  item->font().setWeight(3);

  ui->apList->setItem(row, COLUMN_SSID, item);

  item = new QTableWidgetItem(ap.value("MAC"));
  ui->apList->setItem(row, COLUMN_MAC, item);

  item = new QTableWidgetItem(" | " + ap.value("STR"));
  item->setIcon(QIcon(getIconWifiStr(ap.value("STR").toInt())));
  ui->apList->setItem(row, COLUMN_STR, item);

  item = new QTableWidgetItem(ap.value("FREQ"));
  ui->apList->setItem(row, COLUMN_FREQ, item);

  item = new QTableWidgetItem(ap.value("PATH"));
  ui->apList->setItem(row, COLUMN_PATH, item);
}

void MainWindow::apRemove(QMap<QString, QString> ap) {
  /*Unused - just checking how it works*/
  QList<int> del = getApIndexByPath(ap);
  if(del.count() == 0) return;

  for(int i = del.count() - 1; i >= 0; --i) {
    ui->apList->removeRow(i);
  }
}

void MainWindow::apAdd(QMap<QString, QString> ap){
  /*Unused - just checking how it works*/
  QList<int> count = getApIndexByPath(ap);
  if(count.count() > 0) return;

  insertApToTable(ap);
}

QList<int> MainWindow::getApIndexByPath(const QMap<QString, QString> &ap) {
  /*Unused - just checking how it works*/
  QTableWidgetItem *item;
  QList<int> rows;

  for(int i = 0; i < ui->apList->rowCount(); ++i) {
     item = ui->apList->item(i, COLUMN_PATH);
     if(item->text() == ap.value("PATH")) {
        rows.append(i);
     }
  }

  return rows;
}

void MainWindow::on_refreshBtn_clicked()
{
  if(nm->dbusConnected() == false) {
    QMessageBox::critical(this, "Błąd inicjalizacji", "DBus nie jest poprawnie podłączony");
    return;
  }
  ui->quickWidget->show();
  nm->refreshOnce();
}

void MainWindow::on_updateActivation_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked) {
      on_refreshBtn_clicked();
    } else {
      timer.stop();
    }
}

void MainWindow::timerTimeOut() {
  if(ui->updateActivation->checkState() == Qt::Checked) {
    on_refreshBtn_clicked();
  }
}

void MainWindow::cancelScan() {
  if(ui->updateActivation->checkState() == Qt::Checked) {
    timer.start();
  } else {
    ui->quickWidget->close();
    ui->statusBar->showMessage("Proszę odczekać przynajmniej 10 sekund między skanami", 3000);
  }
}

QString MainWindow::getIconWifiStr(int apStr) {
  if(apStr == 0) return "Icons/wifi-none.png";
  if(apStr < 20) return "Icons/wifi-0.png";
  if(apStr < 45) return "Icons/wifi-1.png";
  if(apStr < 70) return "Icons/wifi-2.png";
  return "Icons/wifi.png";
}
