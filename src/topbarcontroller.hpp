// include/topbarcontroller.hpp
#pragma once

#include <QObject>
#include <QWindow>
#include <QTimer>
#include <QOperatingSystemVersion>
#include "menucontroller.hpp"

#ifdef Q_OS_WIN
#include <wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")
#endif

class TopbarController : public QObject {
    Q_OBJECT
    Q_PROPERTY(MenuController* menuController READ menuController CONSTANT)
    Q_PROPERTY(bool isEthernet READ isEthernet NOTIFY networkChanged)
    Q_PROPERTY(int wifiStrength READ wifiStrength NOTIFY networkChanged)
    Q_PROPERTY(bool isOnBattery READ isOnBattery NOTIFY batteryChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryChanged)
    Q_PROPERTY(bool blur_supported READ blurSupported CONSTANT)
    Q_PROPERTY(bool windowVisible READ windowVisible WRITE setWindowVisible NOTIFY windowVisibilityChanged)

public:
    explicit TopbarController(QObject* parent = nullptr);
    ~TopbarController();

    MenuController* menuController() const { return m_menuController; }
    bool isEthernet() const { return m_isEthernet; }
    int wifiStrength() const { return m_wifiStrength; }
    bool isOnBattery() const { return m_isOnBattery; }
    int batteryLevel() const { return m_batteryLevel; }
    bool blurSupported() const { return m_blurSupported; }
    bool windowVisible() const { return m_windowVisible; }
    void setWindowVisible(bool visible);

public slots:
    void initialize(QWindow* window);
    void cleanup();
    void openSettings();
    void openTaskManager();
    void exitApp();

signals:
    void windowPosChanged(int x, int y, int width, int height);
    void networkChanged();
    void batteryChanged();
    void windowVisibilityChanged();

private slots:
    void checkNetworkStatus();
    void checkBatteryStatus();

private:
    void setupAppbar();
    void enableBlur();
    bool initializeWMI();
    void cleanupWMI();
    bool checkEthernetStatus();
    bool checkWiFiStatus(int& strength);
    bool getBatteryInfo(bool& onBattery, int& level);

private:
    MenuController* m_menuController;
    QWindow* m_window;
    QTimer* m_networkTimer;
    QTimer* m_batteryTimer;
    const int m_topbarHeight = 30;

    bool m_isEthernet;
    int m_wifiStrength;
    bool m_isOnBattery;
    int m_batteryLevel;
    bool m_blurSupported;
    bool m_windowVisible = true;

#ifdef Q_OS_WIN
    IWbemLocator* m_wbemLocator;
    IWbemServices* m_wbemServices;
#endif
};
