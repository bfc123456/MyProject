import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Settings 2.2

Item {
    id: root
    width: parent.width
    height: inputPanel.height

    InputPanel {
        id: inputPanel
        z: 99
        width: root.width
        visible: true

        states: State {
            name: "visible"
            when: inputPanel.active
            PropertyChanges {
                target: inputPanel
                y: root.height-inputPanel.height
            }
        }
        transitions: Transition {
            from: ""
            to: "visible"
            reversible: true
            ParallelAnimation {
                NumberAnimation {
                    properties: "y"
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }

//        Component.onCompleted: {
//            VirtualKeyboardSettings.styleName = "retro"; // 复古样式，可根据需要修改
//            VirtualKeyboardSettings.wordCandidateList.alwaysVisible = true;
//            VirtualKeyboardSettings.activeLocales = ["en_US", "zh_CN", "ja_JP"];
//        }
    }
}
