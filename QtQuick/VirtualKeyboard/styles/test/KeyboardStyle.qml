import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Styles 2.2

KeyboardStyle {
    // 主题属性（保持不变）
    readonly property color panelColor: "#0B1220"
    readonly property color keyColor: "#2F333A"
    readonly property color keyHover: "#3A4048"
    readonly property color keyDown: "#556270"
    readonly property color keyText: "#EDEFF5"
    readonly property color keyBorder: "#00000066"
    readonly property real radius: 10
    readonly property int gap: 8

    // 整板背景
    keyboardBackground: KeyboardBackground {
        color: panelColor
    }

    // 按键面板：通过自定义 Key 组件实现样式
    keyPanel: Item {
        anchors.fill: parent
        anchors.margins: gap

        // 遍历键盘布局，为每个按键创建样式
        Repeater {
            model: KeyboardLayout.model

            delegate: Key {
                // 按键背景：通过 Rectangle 作为子组件实现
                Rectangle {
                    id: keyBg
                    anchors.fill: parent
                    radius: parent.radius
                    color: control.pressed ? keyDown : (control.hovered ? keyHover : keyColor)
                    border.color: keyBorder
                    border.width: 1
                }

                // 按键文字：通过 Text 作为子组件实现
                Text {
                    id: keyLabel
                    anchors.centerIn: parent
                    text: control.displayText
                    color: keyText
                    font.family: control.font.family
                    font.pixelSize: control.font.pixelSize
                }
            }
        }
    }

    Component.onCompleted: {
        console.log("自定义样式 'test' 已加载");
    }
}
