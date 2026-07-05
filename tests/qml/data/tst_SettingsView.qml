import QtQuick
import QtTest

Item {
    id: root
    width: 800
    height: 600

    QtObject {
        id: settingsVm
        property string theme: "dark"
        property int audioQuality: 2
        property string downloadPath: "/home/user/Music"
        property bool isNeteaseLoggedIn: false
        property string neteaseUsername: ""
        property bool hasError: false
        property var error: ({ message: "", type: 0 })

        property int logoutCount: 0
        property int clearHistoryCount: 0
        property string lastTheme: ""
        property int lastAudioQuality: -1
        property string lastCookie: ""

        function setTheme(t) {
            lastTheme = t
            theme = t
        }
        function setAudioQuality(q) {
            lastAudioQuality = q
            audioQuality = q
        }
        function setDownloadPath(p) {
            downloadPath = p
        }
        function logoutNetease() {
            logoutCount += 1
            isNeteaseLoggedIn = false
            neteaseUsername = ""
        }
        function clearPlayHistory() {
            clearHistoryCount += 1
        }
        function clearError() {
            hasError = false
            error = ({ message: "", type: 0 })
        }
        function importNeteaseCookie(cookie) {
            lastCookie = cookie
        }
    }

    QtObject {
        id: mainVm
        property int currentView: 5
        function navigateTo(view) {}
    }

    TestCase {
        name: "SettingsView"
        when: windowShown

        function createView(props) {
            var component = Qt.createComponent("../../../src/qml/SettingsView.qml")
            verify(component.status === Component.Ready, component.errorString())
            var instance = component.createObject(root, props || {})
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

        function test_component_loads() {
            var instance = createView()
            instance.destroy()
        }

        function test_logged_out_action_label() {
            settingsVm.isNeteaseLoggedIn = false
            settingsVm.neteaseUsername = ""

            var instance = createView()
            waitForRendering(instance)

            var button = findObject(instance, "accountActionButton")
            verify(button !== null)
            compare(button.text, "Import Cookie")

            instance.destroy()
        }

        function test_clear_session_calls_vm() {
            settingsVm.isNeteaseLoggedIn = true
            settingsVm.neteaseUsername = "TestUser"
            settingsVm.logoutCount = 0

            var instance = createView()
            waitForRendering(instance)

            var button = findObject(instance, "accountActionButton")
            verify(button !== null)

            button.clicked()
            compare(settingsVm.logoutCount, 1)

            instance.destroy()
        }

        function test_clear_history_calls_vm() {
            settingsVm.clearHistoryCount = 0

            var instance = createView()
            waitForRendering(instance)

            var button = findObject(instance, "clearHistoryButton")
            verify(button !== null)
            button.clicked()

            compare(settingsVm.clearHistoryCount, 1)

            instance.destroy()
        }
    }
}
