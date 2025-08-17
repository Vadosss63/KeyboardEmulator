#include <QApplication>

#include "KeyboardController.h"
#include "MainWindow.h"
#include "SerialPortModel.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    MainWindow         w;
    SerialPortModel    model;
    KeyboardController controller(&model, &w);

    w.show();

    return app.exec();
}
