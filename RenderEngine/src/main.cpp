#include <QApplication>
#include "Widget/ViewWrapper.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    GLRhi::ViewWrapper window;
    window.resize(1800, 1400);
    window.show();

    return app.exec();
}