#include "mainwindow.hpp"
#include "gammaargsdialog.hpp"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<std::shared_ptr<QImage>>("std::shared_ptr<QImage>");
    qRegisterMetaType<std::shared_ptr<const QImage>>("std::shared_ptr<const QImage>");
    qRegisterMetaType<std::function<void(QImage&)>>("std::function<void(QImage&)>");
    qRegisterMetaType<HistRGB>("HistRGB");

    QApplication a(argc, argv);

    QFile styleF;
    styleF.setFileName(":/styles/style.css");
    styleF.open(QFile::ReadOnly);

    a.setStyleSheet(styleF.readAll());

    MainWindow w;
    w.show();

    return a.exec();
}
