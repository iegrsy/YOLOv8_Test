#include "classificationtools.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	ClassificationTools w;		//class called w
	w.show();
    return a.exec();
}
