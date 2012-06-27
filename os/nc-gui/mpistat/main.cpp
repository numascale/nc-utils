#include "mpistat.hpp"
#include <QtGui/QApplication>
#include <string>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	std::string strCacheAddr;
	std::string strMpiAddr;

	if( argc > 1 ) {
		for( int i = 1; i < argc; i++ ) {
			if( strcmp(argv[i], "-cache") == 0 ) {
				if( argc > i + 1 ) {
					strCacheAddr = std::string(argv[i + 1]);
					++i;
				}
			}
			if( strcmp(argv[i], "-mpi") == 0 ) {
				if( argc > i + 1 ) {
					strMpiAddr = std::string(argv[i + 1]);
					++i;
				}
			}
		}			
	}

	mpistat w(strCacheAddr, strMpiAddr);

	w.show();
	return a.exec();
}
