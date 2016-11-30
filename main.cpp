#include "apm.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    APM w;
    w.show();

    return a.exec();
}
