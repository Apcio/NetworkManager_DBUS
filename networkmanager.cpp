#include "networkmanager.h"

NetworkManager::NetworkManager(QObject *_parent): QObject(_parent) {
  initializeInterface();
}

NetworkManager::~NetworkManager() {
  delete nm_interface;
  QList<QDBusInterface *>::iterator it;
  for(it = wifi_intefaces.begin(); it != wifi_intefaces.end(); ++it) {
    delete (*it);
  }
}

bool NetworkManager::dbusConnected() {
  return QDBusConnection::systemBus().isConnected() == true && nm_interface->isValid() == true;
}

bool NetworkManager::refreshOnce() {
  if(dbusConnected() == false) return false;
  QList<QDBusInterface *>::iterator it;
  QMap<QString, QVariant> argList;

  for(it = wifi_intefaces.begin(); it != wifi_intefaces.end(); ++it) {
    if((*it)->isValid() == true) {
      QDBusMessage msg = (*it)->call("RequestScan", argList);

      if(msg.type() == QDBusMessage::ErrorMessage) {
        emit scanError();
      } else {
        QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", (*it)->path(), "org.freedesktop.DBus.Properties",
                                             "PropertiesChanged", this, SLOT(accessPointRefreshed(QDBusMessage)));
      }
    }
  }

  return true;
}

void NetworkManager::initializeInterface() {
  nm_interface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                                    "org.freedesktop.NetworkManager", QDBusConnection::systemBus());

  if(dbusConnected() == false) return;
  if(getWifiInterfaces() == false) return;
}

bool NetworkManager::getWifiInterfaces() {
  QDBusMessage msg = nm_interface->call("GetDevices");
  if(msg.arguments().count() == 0) {
    return false;
  }

  QDBusArgument arg = msg.arguments().at(0).value<QDBusArgument>();
  if(arg.currentType() != QDBusArgument::ArrayType) {
    return false;
  }

  wifi_intefaces.clear();

  QList<QDBusObjectPath> pList = qdbus_cast<QList<QDBusObjectPath>>(arg);
  for(const QDBusObjectPath &p : pList) {
    QDBusInterface device("org.freedesktop.NetworkManager", p.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
    if(device.property("DeviceType") != NM_DEVICE_TYPE_WIFI) {
      continue;
    }

    wifi_intefaces.append(new QDBusInterface("org.freedesktop.NetworkManager", p.path(), "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus()));
  }
  return true;
}

void NetworkManager::disconnectFromDbusSignals(QString path) {
  /*Unused - just checking how it works*/
  QDBusConnection::systemBus().disconnect("org.freedesktop.NetworkManager", path, "org.freedesktop.NetworkManager.Device.Wireless",
                                          "AccessPointAdded", this, SLOT(accessPointChange(QDBusMessage)));
  QDBusConnection::systemBus().disconnect("org.freedesktop.NetworkManager", path, "org.freedesktop.NetworkManager.Device.Wireless",
                                          "AccessPointRemoved", this, SLOT(accessPointChange(QDBusMessage)));
}


void NetworkManager::connectToDbusSignals(QString path) {
  /*Unused - just checking how it works*/
  QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", path, "org.freedesktop.NetworkManager.Device.Wireless",
                                       "AccessPointAdded", this, SLOT(accessPointChange(QDBusMessage)));
  QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", path, "org.freedesktop.NetworkManager.Device.Wireless",
                                       "AccessPointRemoved", this, SLOT(accessPointChange(QDBusMessage)));
}

void NetworkManager::getAllAccessPoint(QDBusInterface *wi) {
  if(wi->isValid() == false) return;

  QDBusMessage msg = wi->call("GetAllAccessPoints");
  if(msg.arguments().count() == 0) return;

  QList<QMap<QString, QString>> aps;

  QDBusArgument apList = msg.arguments().at(0).value<QDBusArgument>();
  QList<QDBusObjectPath> apPathList = qdbus_cast<QList<QDBusObjectPath>>(apList);
  for(const QDBusObjectPath &p : apPathList) {
    QMap<QString, QString> ap = getApInfo(p);
    if(ap.contains("SSID")) aps.append(ap);
  }

  if(aps.count() > 0) emit apRefreshAll(aps);
}

QMap<QString, QString> NetworkManager::getApInfo(QDBusObjectPath path) {
  QMap<QString, QString> ap;
  QDBusInterface api("org.freedesktop.NetworkManager", path.path(), "org.freedesktop.NetworkManager.AccessPoint", QDBusConnection::systemBus());
  if(api.isValid() == true) {
    ap.insert("SSID", api.property("Ssid").toString());
    ap.insert("FREQ", api.property("Frequency").toString());
    ap.insert("MAC", api.property("HwAddress").toString());
    ap.insert("STR", QString::number(api.property("Strength").toInt()));
    ap.insert("PATH", path.path());
  }

  return ap;
}

void NetworkManager::accessPointChange(QDBusMessage msg) {
  /*Unused - just checking how it works*/
  if(msg.type() != QDBusMessage::SignalMessage) return;
  QDBusObjectPath p = msg.arguments().at(0).value<QDBusObjectPath>();
  if(p.path() == "") return;

  QMap<QString, QString> ap = getApInfo(p);

  if(msg.member() == "AccessPointAdded") emit apAdd(ap);
  if(msg.member() == "AccessPointRemoved") emit apRemove(ap);
}

void NetworkManager::accessPointRefreshed(QDBusMessage msg) {
  if(msg.type() != QDBusMessage::SignalMessage) return;
  if(msg.path() == "") return;

  QList<QDBusInterface *>::iterator it;
  for(it = wifi_intefaces.begin(); it != wifi_intefaces.end(); ++it) {
    if((*it)->path() == msg.path()) {
      getAllAccessPoint(*it);
      QDBusConnection::systemBus().disconnect("org.freedesktop.NetworkManager", (*it)->path(), "org.freedesktop.DBus.Properties",
                                              "PropertiesChanged", this, SLOT(accessPointRefreshed(QDBusMessage)));
    }
  }
}
