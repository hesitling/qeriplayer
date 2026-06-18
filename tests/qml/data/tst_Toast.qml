import QtQuick
import QtTest

Item {
    id: root
    width: 400
    height: 100

    TestCase {
        name: "Toast"
        when: windowShown

        function test_initial_state() {
            var toast = Qt.createComponent("../../../src/qml/Toast.qml")
            verify(toast.status === Component.Ready, "Component should load: " + toast.errorString())

            var instance = toast.createObject(root)
            verify(instance !== null, "Object should be created")

            // Verify initial state
            compare(instance.visible, false)
            compare(instance.message, "")
            compare(instance.autoHideMs, 4000)

            instance.destroy()
        }

        function test_show_sets_message() {
            var toast = Qt.createComponent("../../../src/qml/Toast.qml")
            verify(toast.status === Component.Ready, "Component should load: " + toast.errorString())

            var instance = toast.createObject(root)
            verify(instance !== null, "Object should be created")

            // Show toast
            instance.show("Test error message")

            // Verify message is set
            compare(instance.message, "Test error message")

            instance.destroy()
        }
    }
}
