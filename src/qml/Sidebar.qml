import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: sidebar
    color: Material.dialogColor

    property int currentView: mainVm ? mainVm.currentView : 0

    ListModel {
        id: navModel
        ListElement { name: "Home"; icon: "🏠"; view: 0 }
        ListElement { name: "Search"; icon: "🔍"; view: 1 }
        ListElement { name: "Library"; icon: "📚"; view: 2 }
        ListElement { name: "Settings"; icon: "⚙"; view: 5 }
    }

    ListView {
        id: navList
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4
        model: navModel

        delegate: ItemDelegate {
            width: navList.width
            height: 48
            highlighted: sidebar.currentView === model.view

            contentItem: RowLayout {
                spacing: 12

                Label {
                    text: model.icon
                    font.pixelSize: 20
                }

                Label {
                    text: model.name
                    Layout.fillWidth: true
                    font.pixelSize: 14
                    color: highlighted ? Material.accentColor : Material.foreground
                }
            }

            onClicked: {
                mainVm.navigateTo(model.view)
            }
        }
    }
}
