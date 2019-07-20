#include "mainwindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile styleF;

    styleF.setFileName(":/styles/style.css");
    styleF.open(QFile::ReadOnly);

    a.setStyleSheet(styleF.readAll());

    MainWindow w;
    w.show();

    return a.exec();
}
