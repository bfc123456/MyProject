import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.3

Rectangle {
    anchors.fill: parent
    radius: 8
    color: control.pressed ? KeyboardStyle.keyColorPressed : KeyboardStyle.keyColor
    border.color: KeyboardStyle.keyBorderColor
    border.width: 0
}
