/*
 * Copyright (C) 2011 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     donghualin <donghualin@uniontech.com>
 *
 * Maintainer: donghualin <donghualin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEVICEINTERREALIZE_H
#define DEVICEINTERREALIZE_H

#include "netinterface.h"
#include "networkconst.h"
#include "netutils.h"

#include <QJsonArray>
#include <QObject>

namespace dde {

namespace network {

#define MaxQueueSize 4

class DeviceInterRealize : public NetworkDeviceRealize
{
    Q_OBJECT

    friend class NetworkInterProcesser;
    friend class NetworkDeviceBase;

public:
    inline bool isEnabled() const override { return m_enabled; }                                             // 当前的网卡是否启用
    inline QString interface() const override { return m_data.value("Interface").toString(); }               // 返回设备上的Interface
    inline QString driver() const override { return m_data.value("Driver").toString(); }                     // 驱动，对应于备上返回值的Driver
    inline bool managed() const override { return m_data.value("Managed").toBool(); }                        // 对应于设备上返回值的Managed
    inline QString vendor() const override { return m_data.value("Vendor").toString(); }                     // 对应于设备上返回值的Vendor
    inline QString uniqueUuid() const override { return m_data.value("UniqueUuid").toString(); }             // 网络设备的唯一的UUID，对应于设备上返回值的UniqueUuid
    inline bool usbDevice() const override { return m_data.value("UsbDevice").toBool(); }                    // 是否是USB设备，对应于设备上返回值的UsbDevice
    inline QString path() const override { return m_data.value("Path").toString(); }                         // 设备路径，对应于设备上返回值的Path
    inline QString activeAp() const override { return m_data.value("ActiveAp").toString(); }                 // 对应于设备上返回值的ActiveAp
    inline bool supportHotspot() const override { return m_data.value("SupportHotspot").toBool(); }          // 是否支持热点,对应于设备上返回值的SupportHotspot
    inline QString realHwAdr() const override { return m_data.value("HwAddress").toString(); }               // mac地址
    inline QString usingHwAdr() const override { return m_data.value("ClonedAddress").toString(); }          // 正在使用的mac地址
    const QStringList ipv4() override;                                                                       // IPV4地址
    const QStringList ipv6() override;                                                                       // IPV6地址
    QJsonObject activeConnectionInfo() const override;                                                       // 获取当前活动连接的信息
    void setEnabled(bool enabled) override;                                                                  // 开启或禁用网卡
    Connectivity connectivity();

protected:
    explicit DeviceInterRealize(IPConfilctChecker *ipChecker, NetworkInter *networkInter, QObject *parent = Q_NULLPTR);
    virtual ~DeviceInterRealize() override;
    NetworkInter *networkInter();
    void updateDeviceInfo(const QJsonObject &info);
    void initDeviceInfo();
    QStringList getValidIPV4(const QStringList &ipv4s);
    bool isIpv4Address(const QString &ip) const;
    virtual bool isConnected() const = 0;                                                                   // 当前网络的网络是否处于连接状态
    virtual void updateConnection(const QJsonArray &info) = 0;
    virtual QString deviceKey() = 0;                                                                        // 返回设备对应的key值
    virtual void setDeviceEnabledStatus(const bool &enabled);
    virtual void updateActiveInfo(const QList<QJsonObject> &info);                                          // 当前连接发生变化，例如从一个连接切换到另外一个连接
    virtual void updateActiveConnectionInfo(const QList<QJsonObject> &infos);                               // 当前连接发生变化后，获取设备的活动信息，例如IP等

private:
    NetworkInter *m_networkInter;
    QJsonObject m_data;
    QJsonObject m_activeInfoData;
    bool m_enabled;
    Connectivity m_connectivity;
    QQueue<DeviceStatus> m_statusQueue;
    QString m_name;
};

class WiredDeviceInterRealize : public DeviceInterRealize
{
    Q_OBJECT

    friend class NetworkInterProcesser;

private:
    WiredDeviceInterRealize(IPConfilctChecker *ipChecker, NetworkInter *networkInter, QObject *parent);
    ~WiredDeviceInterRealize() override;

public:
    bool connectNetwork(WiredConnection *connection) override;                                              // 连接网络，连接成功抛出deviceStatusChanged信号
    void disconnectNetwork() override;                                                                      // 断开网络连接
    QList<WiredConnection *> wiredItems() const override;                                                   // 有线网络连接列表

private:
    bool isConnected() const override;                                                                      // 是否连接网络，重写基类的虚函数
    void updateConnection(const QJsonArray &info) override;
    void updateActiveInfo(const QList<QJsonObject> &info) override;
    QString deviceKey() override;
    WiredConnection *findConnection(const QString &path);
    WiredConnection *findWiredConnectionByUuid(const QString &uuid);
    void setDeviceEnabledStatus(const bool &enabled) override;

private:
    QList<WiredConnection *> m_connections;
};

class WirelessDeviceInterRealize : public DeviceInterRealize
{
    Q_OBJECT

    friend class NetworkInterProcesser;

public:
    QList<AccessPoints *> accessPointItems() const override;                                                // 当前网卡上所有的网络列表
    void scanNetwork() override;                                                                            // 重新加载所有的无线网络列表
    void connectNetwork(const AccessPoints *item) override;                                                 // 连接网络，连接成功抛出deviceStatusChanged信号
    void disconnectNetwork() override;                                                                      // 断开连接
    QList<WirelessConnection *> items() const;                                                              // 无线网络连接列表
    AccessPoints *activeAccessPoints() const override;                                                      // 当前活动的无线连接

protected:
    WirelessDeviceInterRealize(IPConfilctChecker *ipChecker, NetworkInter *networkInter, QObject *parent);
    ~WirelessDeviceInterRealize() override;

private:
    bool isConnected() const override;                                                                      // 是否连接网络，重写基类的虚函数
    AccessPoints *findAccessPoint(const QString &ssid);
    WirelessConnection *findConnectionByAccessPoint(const AccessPoints *accessPoint);
    void syncConnectionAccessPoints();
    void updateActiveInfo();
    QList<WirelessConnection *> wirelessItems() const override;                                             // 无线网络连接列表

protected:
    void updateConnection(const QJsonArray &info) override;
    void createConnection(const QJsonArray &info);
    void updateActiveInfo(const QList<QJsonObject> &info) override;
    QString deviceKey() override;
    WirelessConnection *findConnectionByPath(const QString &path);
    void updateAccesspoint(const QJsonArray &json);
    void setDeviceEnabledStatus(const bool &enabled) override;
    void updateActiveConnectionInfo(const QList<QJsonObject> &infos) override;
    bool hotspotEnabled() override;

    template<class T>
    void clearListData(QList<T *> &dataList) {
        for (T *data : dataList)
            delete data;

        dataList.clear();
    }

private:
    QList<WirelessConnection *> m_connections;
    QList<AccessPoints *> m_accessPoints;
    QJsonObject m_activeHotspotInfo;
    QList<QJsonObject> m_activeAccessPoints;
    QJsonObject m_hotspotInfo;
    QJsonArray m_connectionJson;
};

}

}

#endif // INTERDEVICE_H
