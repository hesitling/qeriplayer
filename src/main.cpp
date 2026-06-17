#include "app/NeriPlayerApplication.h"

#include <QDebug>
#include <exception>

int main(int argc, char *argv[])
{
    try {
        NeriPlayerQt::NeriPlayerApplication app(argc, argv);
        if (!app.initialize()) {
            return 1;
        }

        app.showMainWindow();
        return app.exec();
    } catch (const std::exception &ex) {
        qCritical() << "Unhandled exception:" << ex.what();
        return 1;
    } catch (...) {
        qCritical() << "Unhandled unknown exception";
        return 1;
    }
}
