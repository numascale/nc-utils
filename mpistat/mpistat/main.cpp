#include "mpistat.hpp"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	mpistat w;
	w.show();
	return a.exec();
}
