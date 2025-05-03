// qml/topbar.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import "." as Local

Window {
    id: topbarWindow
    visible: true
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"  // Make transparent to show background effect

    // Font loaders
    FontLoader { id: monaRegular; source: "qrc:/fonts/MonaSans-Regular.ttf" }
    FontLoader { id: monaBold; source: "qrc:/fonts/MonaSans-Bold.ttf" }

    MouseArea {
        anchors.fill: parent
        onClicked: topbarController.handleTopbarClick()
    }

    // Background with noise effect
    Item {
        anchors.fill: parent

        Rectangle {
            id: backgroundRect
            anchors.fill: parent
            color: topbarController.blur_supported ? "#2f1e1e1e" : "#991e1e1e"
        }

        ShaderEffect {
            id: noiseEffect
            anchors.fill: parent
            property real time: 0
            property var source: ShaderEffectSource {
                sourceItem: backgroundRect
                live: true
                hideSource: true
            }
            
            fragmentShader: "
                #version 440
                layout(location = 0) in vec2 qt_TexCoord0;
                layout(location = 0) out vec4 fragColor;
                layout(std140, binding = 0) uniform buf {
                    mat4 qt_Matrix;
                    float qt_Opacity;
                    float time;
                };
                uniform sampler2D source;
                
                float rand(vec2 co) {
                    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
                }
                
                void main() {
                    vec4 baseColor = texture(source, qt_TexCoord0);
                    float noise = rand(qt_TexCoord0 + vec2(time * 0.1));
                    fragColor = baseColor + vec4(vec3(noise), 0) * 0.03;
                }"
        }

        Timer {
            running: true
            repeat: true
            interval: 50
            onTriggered: noiseEffect.time += 0.1
        }
    }

    // Bottom border
    Rectangle {
        id: bottomBorder
        height: 1
        color: "#28949494"
        width: parent.width
        anchors.bottom: parent.bottom
    }

    // Main content
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.bottomMargin: 1  // Account for bottom border
        spacing: 8
        
        Item {
            width: 20
            height: 20
            Layout.alignment: Qt.AlignVCenter

            Image {
                id: logoImage
                width: 18
                height: 18
                source: "qrc:/vector/logo.svg"
                anchors.fill: parent
                mipmap: true
                opacity: logoArea.containsMouse ? 0.8 : 1.0
            }

            MouseArea {
                id: logoArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    if (logoMenu.visible) {
                        logoMenu.hide()
                    } else {
                        var globalPos = logoArea.mapToGlobal(0, 0)
                        logoMenu.show(globalPos.x, globalPos.y + height)
                    }
                }
            }
        
            Local.SystemMenu {
                id: logoMenu
                visible: false
            }
        }
        
        // Active Process Name
        Label {
            text: menuController.activeApp
            color: "white"
            font.pixelSize: 13
            font.family: monaBold.name
            font.weight: Font.Bold
            Layout.leftMargin: 4
            Layout.alignment: Qt.AlignVCenter
            
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 0.8
                onExited: parent.opacity = 1.0
            }
        }

        // First Separator
        Rectangle {
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            Layout.preferredWidth: 1
            Layout.preferredHeight: 14
            Layout.alignment: Qt.AlignVCenter
            color: "#ffffff"
            opacity: 0.2
        }
        
        // Menu Items
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignVCenter
            clip: true

            ListView {
                id: menuListView
                orientation: ListView.Horizontal
                spacing: 24
                model: menuController.mainMenu.items

                delegate: Label {
                    text: modelData.text
                    color: "white"
                    font.pixelSize: 13
                    font.family: monaRegular.name
                    font.weight: Font.Normal
                    opacity: enabled ? (menuArea.containsMouse ? 1.0 : 0.9) : 0.5
                    enabled: modelData.state !== 1
                    height: menuListView.height
                    verticalAlignment: Text.AlignVCenter
                    
                    MouseArea {
                        id: menuArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                        onClicked: {
                            if (enabled) {
                                menuController.triggerMenuItem(modelData.text)
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: "#ffffff"
                            opacity: parent.containsMouse && parent.enabled ? 0.1 : 0
                            radius: 3
                        }
                    }
                }
            }
        }

        // Right side system indicators
        Row {
            spacing: 16
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            
            // Battery Indicator
            Rectangle {
                id: batteryIndicator
                width: 24
                height: 13
                visible: topbarController.isOnBattery
                color: "transparent"
                border.color: "#ffffff"
                border.width: 1
                radius: 2
                anchors.verticalCenter: parent.verticalCenter
                
                Rectangle {
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    width: (parent.width - 4) * (topbarController.batteryLevel / 100)
                    height: parent.height - 4
                    color: topbarController.batteryLevel > 20 ? "#4cd964" : "#ff3b30"
                    radius: 1
                }
                
                Rectangle {
                    anchors.left: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 3
                    height: 6
                    color: "#ffffff"
                    radius: 1
                }
            }
            
            // WiFi Indicator
            Item {
                width: 16
                height: 16
                anchors.verticalCenter: parent.verticalCenter
            
                Image {
                    id: networkIcon
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: {
                        if (topbarController.isEthernet) {
                            return "qrc:/vector/ethernet.svg"
                        } else {
                            switch(topbarController.wifiStrength) {
                                case 1: return "qrc:/vector/wifi_lv1.svg"
                                case 2: return "qrc:/vector/wifi_lv2.svg"
                                case 3: return "qrc:/vector/wifi_lv3.svg"
                                case 4: return "qrc:/vector/wifi_lv4.svg"
                                default: return "qrc:/vector/wifi_lv1.svg"
                            }
                        }
                    }
                    mipmap: true
                    opacity: 0.9
                }
            }

            // Separator
            Rectangle {
                width: 1
                height: 14
                color: "#ffffff"
                opacity: 0.2
                anchors.verticalCenter: parent.verticalCenter
            }
            
            // Date
            Label {
                id: dateLabel
                color: "white"
                font.pixelSize: 12
                font.family: monaRegular.name
                anchors.verticalCenter: parent.verticalCenter
                
                Timer {
                    interval: 1000
                    running: true
                    repeat: true
                    triggeredOnStart: true
                    onTriggered: {
                        var date = new Date();
                        var days = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
                        var months = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];
                        dateLabel.text = days[date.getDay()] + ', ' + months[date.getMonth()] + ' ' + date.getDate();
                    }
                }
            }

            // Separator
            Rectangle {
                width: 1
                height: 14
                color: "#ffffff"
                opacity: 0.2
                anchors.verticalCenter: parent.verticalCenter
            }
            
            // Clock
            Label {
                id: clockLabel
                color: "white"
                font.pixelSize: 12
                font.family: monaRegular.name
                anchors.verticalCenter: parent.verticalCenter
                
                Timer {
                    interval: 1000
                    running: true
                    repeat: true
                    triggeredOnStart: true
                    onTriggered: {
                        var date = new Date();
                        var hours = date.getHours();
                        var minutes = date.getMinutes();
                        var ampm = hours >= 12 ? 'PM' : 'AM';
                        hours = hours % 12;
                        hours = hours ? hours : 12;
                        minutes = minutes < 10 ? '0' + minutes : minutes;
                        clockLabel.text = hours + ':' + minutes + ' ' + ampm;
                    }
                }
            }
        }
    }

    // Handle window position changes
    Connections {
        target: topbarController
        function onWindowPosChanged(x, y, width, height) {
            topbarWindow.x = x
            topbarWindow.y = y
            topbarWindow.width = width
            topbarWindow.height = height
        }
    }

    // Handle menu updates
    Connections {
        target: menuController
        function onMenuChanged(window, app, items) {
            console.log("Menu updated:", app, items.length + " items")
        }
    }

    // Drop shadow for bottom border
    Rectangle {
        id: bottomShadow
        height: 4
        width: parent.width
        anchors.top: bottomBorder.bottom
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#40000000" }
            GradientStop { position: 1.0; color: "#00000000" }
        }
    }

    // Component cleanup
    Component.onDestruction: {
        topbarController.cleanup()
    }
}
