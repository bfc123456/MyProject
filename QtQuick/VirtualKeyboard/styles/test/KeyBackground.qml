import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Styles 2.2

KeyboardStyle {
    // 配色属性
    readonly property color panelColor: "#0F1F3A"    // 面板底色
    readonly property color keyColor: "#2E3440"      // 普通键
    readonly property color keyColorPressed: "#3B4252" // 按下
    readonly property color keyTextColor: "#E6EDF3"  // 字/图标
    readonly property color keyBorderColor: "#00000000" // 键边框

    // 整板背景
    keyboardBackground: KeyboardBackground {
        color: panelColor
    }

    // 按键面板（通过 Repeater 定义单个按键样式）
    keyPanel: Item {
        anchors.fill: parent
        anchors.margins: 8

        Repeater {
            model: KeyboardLayout.model

            delegate: Key {
                // 按键背景
                Rectangle {
                    anchors.fill: parent
                    radius: 10
                    color: control.pressed ? keyColorPressed : keyColor
                    border.color: keyBorderColor
                    border.width: 1
                }

                // 按键文字
                Text {
                    anchors.centerIn: parent
                    text: control.displayText
                    color: keyTextColor
                    font.pixelSize: 22
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    renderType: Text.NativeRendering
                    elide: Text.ElideRight
                }
            }
        }
    }
}
