import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QeriPlayer 1.0

Rectangle {
    id: sidebar
    color: Material.dialogColor

    property int currentView: mainVm ? mainVm.currentView : MainView.Home
    readonly property var navigationItems: [
        { name: "Home", icon: "🏠", view: MainView.Home },
        { name: "Search", icon: "🔍", view: MainView.Search },
        { name: "Library", icon: "📚", view: MainView.Library },
        { name: "Settings", icon: "⚙️", view: MainView.Settings }
    ]

    ListView {
        id: navList
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4
        model: sidebar.navigationItems

        delegate: ItemDelegate {
            width: navList.width
            height: 48
            highlighted: sidebar.currentView === modelData.view

            contentItem: RowLayout {
                spacing: 12
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    text: modelData.icon
                    font.pixelSize: 20
                }

                Label {
                    text: modelData.name
                    Layout.fillWidth: true
                    font.pixelSize: 14
                    color: highlighted ? Material.accentColor : Material.foreground
                }
            }

            onClicked: {
                mainVm.navigateTo(modelData.view)
            }
        }
    }
}
