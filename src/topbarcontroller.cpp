// src/topbarcontroller.cpp
#include "topbarcontroller.hpp"
#include "windowsstructures.hpp"
#include "windowsapi.hpp"
#include <QDebug>
#include <QProcess>
#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <dwmapi.h>
#include <shellapi.h>
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shell32.lib")
#endif

TopbarController::TopbarController(QObject* parent)
    : QObject(parent)
    , m_menuController(new MenuController(this))
    , m_window(nullptr)
    , m_networkTimer(new QTimer(this))
    , m_batteryTimer(new QTimer(this))
    , m_isEthernet(false)
    , m_wifiStrength(4)
    , m_isOnBattery(true)
    , m_batteryLevel(100)
    , m_windowVisible(true)
#ifdef Q_OS_WIN
    , m_wbemLocator(nullptr)
    , m_wbemServices(nullptr)
#endif
{
    m_blurSupported = QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows10;

    m_networkTimer->setInterval(2000);
    connect(m_networkTimer, &QTimer::timeout, this, &TopbarController::checkNetworkStatus);

    m_batteryTimer->setInterval(5000);
    connect(m_batteryTimer, &QTimer::timeout, this, &TopbarController::checkBatteryStatus);

    if (initializeWMI()) {
        m_networkTimer->start();
        m_batteryTimer->start();
    }
}

TopbarController::~TopbarController()
{
    cleanupWMI();
}

bool TopbarController::initializeWMI()
{
#ifdef Q_OS_WIN
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        qDebug() << "Failed to initialize COM";
        return false;
    }

    hr = CoInitializeSecurity(
        nullptr,
        -1,
        nullptr,
        nullptr,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        nullptr,
        EOAC_NONE,
        nullptr
        );

    if (FAILED(hr)) {
        qDebug() << "Failed to initialize security";
        CoUninitialize();
        return false;
    }

    hr = CoCreateInstance(
        CLSID_WbemLocator,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        reinterpret_cast<void**>(&m_wbemLocator)
        );

    if (FAILED(hr)) {
        qDebug() << "Failed to create WbemLocator";
        CoUninitialize();
        return false;
    }

    hr = m_wbemLocator->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        nullptr,
        nullptr,
        nullptr,
        0,
        nullptr,
        nullptr,
        &m_wbemServices
        );

    if (FAILED(hr)) {
        qDebug() << "Failed to connect to WMI";
        m_wbemLocator->Release();
        m_wbemLocator = nullptr;
        CoUninitialize();
        return false;
    }

    return true;
#else
    return false;
#endif
}

void TopbarController::cleanupWMI()
{
#ifdef Q_OS_WIN
    if (m_wbemServices) {
        m_wbemServices->Release();
        m_wbemServices = nullptr;
    }
    if (m_wbemLocator) {
        m_wbemLocator->Release();
        m_wbemLocator = nullptr;
    }
    CoUninitialize();
#endif
}

void TopbarController::initialize(QWindow* window)
{
    m_window = window;
    setupAppbar();

    if (m_blurSupported) {
        enableBlur();
    }

    checkNetworkStatus();
    checkBatteryStatus();
}

void TopbarController::cleanup()
{
#ifdef Q_OS_WIN
    if (m_window) {
        APPBARDATA abd = { sizeof(APPBARDATA) };
        abd.hWnd = reinterpret_cast<HWND>(m_window->winId());
        SHAppBarMessage(ABM_REMOVE, &abd);
    }
#endif
}

void TopbarController::setupAppbar()
{
#ifdef Q_OS_WIN
    if (!m_window) {
        return;
    }

    if (m_blurSupported) {
        m_window->setProperty("_q_stylebackground", true);
    }

    HWND hwnd = reinterpret_cast<HWND>(m_window->winId());
    MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
    HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);

    if (!GetMonitorInfo(monitor, &monitorInfo)) {
        qDebug() << "Failed to get monitor info";
        return;
    }

    const RECT& workArea = monitorInfo.rcWork;
    int width = workArea.right - workArea.left;

    m_window->setX(workArea.left);
    m_window->setY(workArea.top);
    m_window->setWidth(width);
    m_window->setHeight(m_topbarHeight);

    emit windowPosChanged(workArea.left, workArea.top, width, m_topbarHeight);

    APPBARDATA abd = { sizeof(APPBARDATA) };
    abd.hWnd = hwnd;
    abd.uEdge = ABE_TOP;
    abd.rc = {
        workArea.left,
        workArea.top,
        workArea.right,
        workArea.bottom
    };

    SHAppBarMessage(ABM_NEW, &abd);
    SHAppBarMessage(ABM_QUERYPOS, &abd);

    abd.rc.bottom = abd.rc.top + m_topbarHeight;
    SHAppBarMessage(ABM_SETPOS, &abd);
#endif
}

void TopbarController::enableBlur()
{
#ifdef Q_OS_WIN
    if (!m_window || !m_blurSupported) {
        return;
    }

    HWND hwnd = reinterpret_cast<HWND>(m_window->winId());

    HMODULE hUser32 = GetModuleHandle(L"user32.dll");
    if (hUser32) {
        auto setWindowCompositionAttribute = reinterpret_cast<SetWindowCompositionAttribute>(
            GetProcAddress(hUser32, "SetWindowCompositionAttribute")
            );

        if (setWindowCompositionAttribute) {
            ACCENTPOLICY accent = { CustomAccent::ACCENT_ENABLE_BLURBEHIND, 0, 0, 0 };
            WINCOMPATTRDATA data = {
                CustomAccent::WCA_ACCENT_POLICY,
                &accent,
                sizeof(accent)
            };

            setWindowCompositionAttribute(hwnd, &data);
        }
    }
#endif
}

void TopbarController::checkNetworkStatus()
{
#ifdef Q_OS_WIN
    try {
        if (!m_wbemServices) {
            return;
        }

        bool ethernetConnected = false;
        bool wifiConnected = false;
        int wifiStrength = 0;

        // Check ethernet connections first
        IEnumWbemClassObject* pEnumerator = nullptr;
        HRESULT hr = m_wbemServices->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM Win32_NetworkAdapter WHERE NetConnectionStatus = 2 AND PhysicalAdapter = TRUE AND AdapterType = 'Ethernet 802.3'"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr,
            &pEnumerator
        );

        if (SUCCEEDED(hr) && pEnumerator) {
            IWbemClassObject* pclsObj = nullptr;
            ULONG uReturn = 0;

            hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (SUCCEEDED(hr) && uReturn > 0) {
                ethernetConnected = true;
                pclsObj->Release();
            }
            pEnumerator->Release();
        }

        // Check WiFi if no ethernet
        if (!ethernetConnected) {
            pEnumerator = nullptr;
            hr = m_wbemServices->ExecQuery(
                bstr_t("WQL"),
                bstr_t("SELECT * FROM Win32_NetworkAdapter WHERE NetConnectionStatus = 2 AND PhysicalAdapter = TRUE AND NetConnectionID LIKE '%Wi%Fi%'"),
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                nullptr,
                &pEnumerator
            );

            if (SUCCEEDED(hr) && pEnumerator) {
                IWbemClassObject* pclsObj = nullptr;
                ULONG uReturn = 0;

                if (SUCCEEDED(pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn)) && uReturn > 0) {
                    wifiConnected = true;
                    
                    // Get signal strength using GetObject
                    VARIANT vtProp;
                    if (SUCCEEDED(pclsObj->Get(L"Name", 0, &vtProp, 0, 0))) {
                        IWbemClassObject* pSignalObj = nullptr;
                        std::wstring query = L"SELECT * FROM MSNdis_80211_ReceivedSignalStrength WHERE InstanceName = '";
                        query += vtProp.bstrVal;
                        query += L"'";
                        VariantClear(&vtProp);

                        IEnumWbemClassObject* pSignalEnum = nullptr;
                        hr = m_wbemServices->ExecQuery(
                            bstr_t("WQL"),
                            bstr_t(query.c_str()),
                            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                            nullptr,
                            &pSignalEnum
                        );

                        if (SUCCEEDED(hr) && pSignalEnum) {
                            ULONG sigReturn = 0;
                            if (SUCCEEDED(pSignalEnum->Next(WBEM_INFINITE, 1, &pSignalObj, &sigReturn)) && sigReturn > 0) {
                                VARIANT vtSignal;
                                if (SUCCEEDED(pSignalObj->Get(L"Ndis80211ReceivedSignalStrength", 0, &vtSignal, 0, 0))) {
                                    int signal = abs(vtSignal.intVal);
                                    if (signal <= 50) wifiStrength = 4;
                                    else if (signal <= 60) wifiStrength = 3;
                                    else if (signal <= 70) wifiStrength = 2;
                                    else wifiStrength = 1;
                                    VariantClear(&vtSignal);
                                }
                                pSignalObj->Release();
                            }
                            pSignalEnum->Release();
                        }
                    }
                    pclsObj->Release();
                }
                pEnumerator->Release();
            }
        }

        if (m_isEthernet != ethernetConnected || m_wifiStrength != (wifiConnected ? wifiStrength : 0)) {
            m_isEthernet = ethernetConnected;
            m_wifiStrength = wifiConnected ? wifiStrength : 0;
            emit networkChanged();
        }
    }
    catch (const _com_error& e) {
        qDebug() << "WMI error:" << QString::fromWCharArray(e.ErrorMessage());
    }
#endif
}

void TopbarController::checkBatteryStatus()
{
#ifdef Q_OS_WIN
    try {
        SYSTEM_POWER_STATUS powerStatus;
        if (GetSystemPowerStatus(&powerStatus)) {
            bool newIsOnBattery = (powerStatus.ACLineStatus == 0);
            int newBatteryLevel = (powerStatus.BatteryLifePercent == 255) ? 100 : powerStatus.BatteryLifePercent;

            if (m_isOnBattery != newIsOnBattery || m_batteryLevel != newBatteryLevel) {
                m_isOnBattery = newIsOnBattery;
                m_batteryLevel = newBatteryLevel;
                emit batteryChanged();
            }
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Error checking battery status:" << e.what();
    }
#endif
}

void TopbarController::openSettings()
{
    // Launch Windows Settings
    QProcess::startDetached("explorer.exe", QStringList() << "ms-settings:");
}

void TopbarController::openTaskManager()
{
    // Launch Task Manager
    QProcess::startDetached("taskmgr.exe");
}

void TopbarController::exitApp()
{
    QCoreApplication::quit();
}

void TopbarController::setWindowVisible(bool visible)
{
    if (m_windowVisible != visible) {
        m_windowVisible = visible;
        emit windowVisibilityChanged();
    }
}
