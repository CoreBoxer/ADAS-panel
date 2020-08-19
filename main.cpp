#include <QApplication>
#include <QQuickView>
#include <QQmlEngine>
#include "CameraToolBox.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    qmlRegisterType<CameraToolBox>("CameraToolBox", 1, 0, "CameraToolBox");

    QQuickView view;
    view.setSource(QUrl("qrc:/main.qml"));
    app.connect(view.engine(), &QQmlEngine::quit, &QApplication::quit);

    view.show();
    app.exec();
    return 0;
}

