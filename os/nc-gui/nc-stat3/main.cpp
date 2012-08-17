#include "mpistat.hpp"
#include <QtGui/QApplication>
#include <string>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    bool simulate=false;
	std::string strCacheAddr;	
    int nodes;

	if( argc > 1 ) {
		for( int i = 1; i < argc; i++ ) {
			if( strcmp(argv[i], "-cache") == 0 ) {
				if( argc > i + 1 ) {
					strCacheAddr = std::string(argv[i + 1]);
					++i;
				}
			}
            else if( strcmp(argv[i], "-simulate") == 0 ) {
				simulate=true;
                if( argc > i + 1 ) {
                    nodes=atoi(argv[i+1]);
                }

                break;
			}

		}			
	}

	mpistat w(strCacheAddr, simulate, nodes);

	w.show();
	return a.exec();
}
