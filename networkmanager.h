#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H
#include <QtDBus/QtDBus>
#include <QThread>
#include <QtCore/QStringList>

class NetworkManager: public QObject{

    Q_OBJECT

  private:
    const int NM_DEVICE_TYPE_WIFI = 2;
    QDBusInterface *nm_interface;
    QList<QDBusInterface *> wifi_intefaces;

    void initializeInterface();
    void disconnectFromDbusSignals(QString path);
    void connectToDbusSignals(QString path);
    QMap<QString, QString> getApInfo(QDBusObjectPath path);

    void getAllAccessPoint(QDBusInterface *wi);
    bool getWifiInterfaces();

  public:
    NetworkManager(QObject *_parent);
    ~NetworkManager();
    bool dbusConnected();
    bool refreshOnce();

  signals:
    void apRefreshAll(QList<QMap<QString, QString> >);
    void scanError();
    void apAdd(QMap<QString, QString>); /*Unused - just checking how it works*/
    void apRemove(QMap<QString, QString>); /*Unused - just checking how it works*/

  private slots:
    void accessPointChange(QDBusMessage msg); /*Unused - just checking how it works*/
    void accessPointRefreshed(QDBusMessage msg);
};

#endif // NETWORKMANAGER_H
