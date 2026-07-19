/// @file test_main.cpp
/// @brief Qt Quick test runner for QML component tests

#include "app/QmlEnums.h"

#include <QtQuickTest/QtQuickTest>

int main(int argc, char **argv)
{
    QeriPlayerQt::registerQmlEnums();
    return quick_test_main(argc, argv, "qml", QUICK_TEST_SOURCE_DIR);
}
