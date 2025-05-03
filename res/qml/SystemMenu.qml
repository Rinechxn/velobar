// SystemMenu.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Qt5Compat.GraphicalEffects

Window {
    id: menuWindow
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"
    width: 200  // Reduced width
    height: contentColumn.height + 12 // Reduced padding

    // Add property to track menu state
    property bool isOpen: false

    // Add property for main window reference
    property var mainWindow

    // Signal for menu close
    signal menuClosed()

    function show(x, y) {
        menuWindow.x = x
        menuWindow.y = y + 12
        visible = true
        isOpen = true
        requestActivate()
    }

    function hide() {
        menuWindow.visible = false
        isOpen = false
        menuClosed()
        if (mainWindow) {
            mainWindow.visible = true
        }
    }

    Rectangle {
        id: backgroundRect
        anchors.fill: parent
        color: "#1a1a1a"
        radius: 6
        border.color: "#333333"
        border.width: 1

        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            radius: 8.0
            samples: 17
            color: "#80000000"
        }

        ColumnLayout {
            id: contentColumn
            anchors.margins: 6
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            spacing: 2

            Repeater {
                model: [
                    { text: "Settings", action: "settings" },
                    { text: "Task Manager", action: "taskmanager" },
                    { text: "", separator: true },
                    { text: "Shut Down", action: "poweroff" },
                    { text: "Reboot", action: "reboot" },
                    { text: "Exit", action: "exit" }
                ]

                delegate: Item {
                    Layout.fillWidth: true
                    height: modelData.separator ? 1 : 28

                    Rectangle {
                        visible: modelData.separator
                        anchors.fill: parent
                        color: "#333333"
                    }

                    Rectangle {
                        visible: !modelData.separator
                        anchors.fill: parent
                        color: mouseArea.containsMouse ? "#ffffff" : "transparent"
                        radius: 4

                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 8
                            text: modelData.text
                            color: mouseArea.containsMouse ? "#000000" : "#ffffff"
                            font.pixelSize: 12
                            font.family: "Mona Sans"
                            verticalAlignment: Text.AlignVCenter
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                hide()
                                if (mainWindow) {
                                    mainWindow.visible = false
                                }
                                switch(modelData.action) {
                                    case "settings":
                                        topbarController.openSettings()
                                        break
                                    case "taskmanager":
                                        topbarController.openTaskManager()
                                        break
                                    case "exit":
                                        topbarController.exitApp()
                                        break
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    MouseArea {
        id: backgroundArea
        z: -1  // Put behind the menu
        anchors.fill: parent
        anchors.margins: -1000  // Extend well beyond menu bounds
        enabled: menuWindow.visible
        
        onClicked: {
            let clickPos = mapToItem(backgroundRect, mouse.x, mouse.y)
            if (!backgroundRect.contains(clickPos)) {
                hide()
            }
        }
    }
}
