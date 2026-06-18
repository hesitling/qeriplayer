#include "app/QeriPlayerApplication.h"

#include <QDebug>
#include <exception>

int main(int argc, char *argv[])
{
    try {
        QeriPlayerQt::QeriPlayerApplication app(argc, argv);
        if (!app.initialize()) {
            return 1;
        }

        return app.exec();
    } catch (const std::exception &ex) {
        qCritical() << "Unhandled exception:" << ex.what();
        return 1;
    } catch (...) {
        qCritical() << "Unhandled unknown exception";
        return 1;
    }
}
