#include <QApplication>
#include "Widget/ViewWrapper.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    QSurfaceFormat::setDefaultFormat(format);

    GLRhi::ViewWrapper window;
    window.resize(1800, 1400);
    window.show();

    return app.exec();
}