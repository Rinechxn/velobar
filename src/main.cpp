// src/main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>
#include "topbarcontroller.hpp"

int main(int argc, char *argv[])
{
    // Enable high DPI scaling
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("TopbarApp");
    app.setApplicationName("Topbar");

    // Create the controller
    TopbarController controller;

    // Create QML engine
    QQmlApplicationEngine engine;

    // Register the controller instances with QML
    engine.rootContext()->setContextProperty("topbarController", &controller);
    engine.rootContext()->setContextProperty("menuController", controller.menuController());

    // Load the QML file from resources
    engine.load(QUrl(QStringLiteral("qrc:/qml/topbar.qml")));

    // Check if QML loaded successfully
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    // Initialize the controller with the main window
    QWindow* mainWindow = qobject_cast<QWindow*>(engine.rootObjects().first());
    if (mainWindow) {
        controller.initialize(mainWindow);
    }

    // Connect cleanup on app quit
    QObject::connect(&app, &QGuiApplication::aboutToQuit,
                    &controller, &TopbarController::cleanup);

    return app.exec();
}
