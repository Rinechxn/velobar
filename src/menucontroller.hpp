// include/menucontroller.hpp
#pragma once

#include <QObject>
#include <QTimer>
#include <QVariantList>
#include <windows.h>
#include "menuitemmodel.hpp"

class MenuController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString activeWindow READ activeWindow NOTIFY menuChanged)
    Q_PROPERTY(QString activeApp READ activeApp NOTIFY menuChanged)
    Q_PROPERTY(QVariantList menuItems READ menuItems NOTIFY menuChanged)
    Q_PROPERTY(MenuItemModel* mainMenu READ mainMenu CONSTANT)

public:
    explicit MenuController(QObject* parent = nullptr);
    ~MenuController();

    QString activeWindow() const { return m_activeWindow; }
    QString activeApp() const { return m_activeApp; }
    QVariantList menuItems() const { return m_menuItems; }
    MenuItemModel* mainMenu() const { return m_model; }

public slots:
    void triggerMenuItem(const QString& menuText);

signals:
    void menuChanged(const QString& window, const QString& app, const QVariantList& items);

private slots:
    void checkActiveWindow();

private:
    QString getProcessName(HWND hwnd);
    QString getFriendlyAppName(const QString& path, HANDLE processHandle);
    QVariantMap getMenuText(HMENU hmenu, int position);
    QVariantList enumerateMenu(HMENU hmenu, int level = 0);
    QVariantList getWindowMenuItems(HWND hwnd);
    bool triggerMenuItemRecursive(HMENU hmenu, const QString& targetText, int level = 0);

private:
    QString m_activeWindow;
    QString m_activeApp;
    QVariantList m_menuItems;
    QTimer* m_timer;
    HWND m_lastHwnd;
    MenuItemModel* m_model;
};
