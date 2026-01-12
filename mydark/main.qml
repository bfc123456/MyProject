import QtQuick 2.12
import QtQuick.VirtualKeyboard 2.2
import QtQuick.VirtualKeyboard.Styles 2.2

Style {
    name: "mydark"
    InputPanel: Component {
        Item {
            Loader {
                source: "content/InputPanel.qml"
            }
        }
    }
}
