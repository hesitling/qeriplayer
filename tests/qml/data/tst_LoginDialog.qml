import QtQuick
import QtTest

Item {
    id: root
    width: 800
    height: 600

    QtObject {
        id: settingsVm
        property bool isNeteaseLoggedIn: false
        property bool hasError: false
        property var error: ({ message: "", type: 0 })
        property string lastCookie: ""
        property int clearErrorCount: 0

        function clearError() {
            clearErrorCount += 1
            hasError = false
            error = ({ message: "", type: 0 })
        }

        function importNeteaseCookie(cookie) {
            lastCookie = cookie
        }
    }

    TestCase {
        name: "LoginDialog"
        when: windowShown

        function createDialog() {
            var component = Qt.createComponent("../../../src/qml/LoginDialog.qml")
            verify(component.status === Component.Ready, component.errorString())
            var instance = component.createObject(root)
            verify(instance !== null)
            return instance
        }

        function findObject(parent, name) {
            if (!parent)
                return null
            if (parent.objectName === name)
                return parent

            var buckets = []
            if (parent.children)
                buckets.push(parent.children)
            if (parent.contentItem && parent.contentItem.children)
                buckets.push(parent.contentItem.children)

            for (var b = 0; b < buckets.length; b++) {
                var kids = buckets[b]
                for (var i = 0; i < kids.length; i++) {
                    var result = findObject(kids[i], name)
                    if (result)
                        return result
                }
            }
            return null
        }

        function test_import_calls_vm() {
            settingsVm.lastCookie = ""
            settingsVm.clearErrorCount = 0

            var dialog = createDialog()
            dialog.open()
            tryVerify(function() { return dialog.visible })

            var field = findObject(dialog, "cookieField")
            var button = findObject(dialog, "importCookieButton")
            verify(field !== null)
            verify(button !== null)

            field.text = "MUSIC_U=abc; __csrf=xyz"
            button.clicked()

            compare(settingsVm.lastCookie, "MUSIC_U=abc; __csrf=xyz")
            verify(settingsVm.clearErrorCount >= 1)

            dialog.destroy()
        }

        function test_success_closes_dialog() {
            settingsVm.isNeteaseLoggedIn = false

            var dialog = createDialog()
            dialog.open()
            tryVerify(function() { return dialog.visible })

            settingsVm.isNeteaseLoggedIn = true
            tryVerify(function() { return !dialog.visible })

            dialog.destroy()
        }
    }
}
