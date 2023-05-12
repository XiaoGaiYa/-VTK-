#include <QtWidgets/QApplication>
#include "visualdicom.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VisualDicom w;
    w.show();
    return a.exec();
}
