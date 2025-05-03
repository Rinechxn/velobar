// src/menucontroller.cpp
#include "menucontroller.hpp"
#include <QDebug>
#include <psapi.h>
#include <QFileInfo>
#include <QVersionNumber>
#include <QFuture>
#include <memory>

MenuController::MenuController(QObject* parent)
    : QObject(parent)
    , m_lastHwnd(nullptr)
    , m_model(new MenuItemModel(this))
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MenuController::checkActiveWindow);
    m_timer->start(100); // Check every 100ms
}

MenuController::~MenuController()
{
}

QString MenuController::getProcessName(HWND hwnd)
{
    try {
        DWORD processId;
        GetWindowThreadProcessId(hwnd, &processId);

        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
        if (!processHandle) {
            return "Unknown";
        }

        std::unique_ptr<void, decltype(&CloseHandle)> handleGuard(processHandle, CloseHandle);

        wchar_t filePath[MAX_PATH];
        if (GetModuleFileNameEx(processHandle, nullptr, filePath, MAX_PATH)) {
            // First try to get version info description
            DWORD dummy;
            DWORD fileInfoSize = GetFileVersionInfoSize(filePath, &dummy);
            if (fileInfoSize > 0) {
                std::vector<BYTE> fileInfoBuffer(fileInfoSize);
                if (GetFileVersionInfo(filePath, 0, fileInfoSize, fileInfoBuffer.data())) {
                    struct LANGANDCODEPAGE {
                        WORD language;
                        WORD codePage;
                    } *translations;
                    UINT translationsLen = 0;

                    // Get list of languages
                    if (VerQueryValue(fileInfoBuffer.data(), L"\\VarFileInfo\\Translation",
                        (LPVOID*)&translations, &translationsLen)) {
                        
                        // Try different version info strings in order of preference
                        const wchar_t* queries[] = {
                            L"FileDescription",
                            L"ProductName",
                            L"OriginalFilename"
                        };

                        for (const auto& query : queries) {
                            for (UINT i = 0; i < translationsLen / sizeof(LANGANDCODEPAGE); i++) {
                                wchar_t subBlock[128];
                                _snwprintf_s(subBlock, _countof(subBlock), _TRUNCATE,
                                    L"\\StringFileInfo\\%04x%04x\\%s",
                                    translations[i].language,
                                    translations[i].codePage,
                                    query);

                                LPWSTR value = nullptr;
                                UINT len = 0;
                                if (VerQueryValue(fileInfoBuffer.data(), subBlock, (LPVOID*)&value, &len) && value && len > 0) {
                                    QString friendly = QString::fromWCharArray(value).trimmed();
                                    if (!friendly.isEmpty()) {
                                        return friendly;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Fallback to executable name without extension
            QString path = QString::fromWCharArray(filePath);
            return QFileInfo(path).completeBaseName();
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Error getting process name:" << e.what();
    }

    return "Unknown";
}

QVariantMap MenuController::getMenuText(HMENU hmenu, int position)
{
    QVariantMap result;

    try {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
        mii.fMask = MIIM_FTYPE | MIIM_STATE | MIIM_ID;

        if (!GetMenuItemInfo(hmenu, position, TRUE, &mii)) {
            return result;
        }

        // Check if separator
        if (mii.fType & MFT_SEPARATOR) {
            result["text"] = "";
            result["is_separator"] = true;
            result["id"] = (int)mii.wID;
            result["state"] = (int)mii.fState;
            return result;
        }

        // Get text length
        int length = GetMenuString(hmenu, position, nullptr, 0, MF_BYPOSITION);
        if (length == 0) {
            return result;
        }

        // Get text
        std::wstring buffer(length + 1, 0);
        GetMenuStringW(hmenu, position, &buffer[0], length + 1, MF_BYPOSITION);

        // Get submenu
        HMENU submenu = GetSubMenu(hmenu, position);

        // Remove & from text
        QString menuText = QString::fromWCharArray(buffer.c_str()).replace("&", "");

        result["text"] = menuText;
        result["is_separator"] = false;
        result["id"] = (int)mii.wID;
        result["state"] = (int)mii.fState;
        result["has_submenu"] = submenu != nullptr;
        result["submenu_handle"] = (qulonglong)submenu;

        return result;
    }
    catch (const std::exception& e) {
        qDebug() << "Error getting menu text:" << e.what();
        return result;
    }
}

QVariantList MenuController::enumerateMenu(HMENU hmenu, int level)
{
    QVariantList menuItems;

    if (!hmenu) {
        return menuItems;
    }

    try {
        int count = GetMenuItemCount(hmenu);
        if (count == -1) {
            return menuItems;
        }

        for (int i = 0; i < count; ++i) {
            QVariantMap item = getMenuText(hmenu, i);
            if (!item.isEmpty()) {
                item["level"] = level;
                menuItems.append(item);

                if (!item["is_separator"].toBool() && item["has_submenu"].toBool()) {
                    HMENU submenu = (HMENU)item["submenu_handle"].toULongLong();
                    QVariantList subItems = enumerateMenu(submenu, level + 1);
                    menuItems.append(subItems);
                }
            }
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Error enumerating menu:" << e.what();
    }

    return menuItems;
}

QVariantList MenuController::getWindowMenuItems(HWND hwnd)
{
    try {
        HMENU menuBar = GetMenu(hwnd);
        if (menuBar) {
            QVariantList menuItems = enumerateMenu(menuBar);

            // Clean up items for display
            QVariantList cleanedItems;
            for (const QVariant& item : menuItems) {
                QVariantMap itemMap = item.toMap();
                if (!itemMap["text"].toString().isEmpty() || itemMap["is_separator"].toBool()) {
                    itemMap.remove("submenu_handle");
                    cleanedItems.append(itemMap);
                }
            }

            return cleanedItems;
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Error getting window menu items:" << e.what();
    }

    return QVariantList();
}

void MenuController::checkActiveWindow()
{
    try {
        HWND hwnd = GetForegroundWindow();
        if (hwnd && hwnd != m_lastHwnd) {
            m_lastHwnd = hwnd;

            wchar_t windowTitle[256];
            GetWindowTextW(hwnd, windowTitle, 256);
            QString title = QString::fromWCharArray(windowTitle);
            QString processName = getProcessName(hwnd);

            // Get menu items with retry
            QVariantList menuItems;
            int retryCount = 0;
            while (menuItems.isEmpty() && retryCount < 3) {
                menuItems = getWindowMenuItems(hwnd);
                if (menuItems.isEmpty()) {
                    retryCount++;
                    Sleep(50);
                }
            }

            m_activeWindow = title;
            m_activeApp = processName;
            m_menuItems = menuItems;
            m_model->setItems(menuItems);

            // Debug output
            qDebug() << "\nActive Window:" << title;
            qDebug() << "Process:" << processName;
            qDebug() << "Menu Items:" << menuItems.size();

            emit menuChanged(title, processName, menuItems);
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Error checking active window:" << e.what();
    }
}

void MenuController::triggerMenuItem(const QString& menuText)
{
    try {
        HWND hwnd = GetForegroundWindow();
        if (!hwnd) {
            return;
        }

        HMENU menu = GetMenu(hwnd);
        if (menu) {
            if (triggerMenuItemRecursive(menu, menuText)) {
                SetForegroundWindow(hwnd);
            }
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Error triggering menu item:" << e.what();
    }
}

bool MenuController::triggerMenuItemRecursive(HMENU hmenu, const QString& targetText, int level)
{
    try {
        int itemCount = GetMenuItemCount(hmenu);

        for (int i = 0; i < itemCount; ++i) {
            QVariantMap itemInfo = getMenuText(hmenu, i);
            if (itemInfo.isEmpty()) {
                continue;
            }

            if (itemInfo["text"].toString().trimmed() == targetText.trimmed()) {
                HWND hwnd = GetForegroundWindow();
                PostMessage(hwnd, WM_COMMAND, itemInfo["id"].toUInt(), 0);
                return true;
            }

            if (itemInfo["has_submenu"].toBool()) {
                HMENU submenu = (HMENU)itemInfo["submenu_handle"].toULongLong();
                if (triggerMenuItemRecursive(submenu, targetText, level + 1)) {
                    return true;
                }
            }
        }
    }
    catch (const std::exception& e) {
        qDebug() << "Error in recursive menu trigger:" << e.what();
    }

    return false;
}
