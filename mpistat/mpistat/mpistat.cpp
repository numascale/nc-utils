#include "mpistat.hpp"
#include "log2scale.hpp"

#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QBrush>

#include <qwt_symbol.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_engine.h>

const int msgsize = sizeof(uint32_t) + sizeof(struct msgstats_t);

char* getLastErrorMessage(char* buffer, DWORD size, DWORD errorcode);
char* getSockAddrAsString(char* buffer, DWORD size, struct sockaddr* saddr);

static const char* serverName = "narya-02.dyn.int.numascale.com";
static const char* serverPort = "4747"; 

mpistat::mpistat(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
	, timer(this)
	, connected(false)
	, init(true)
{
	ui.setupUi(this);

	graph1 = new SizeHistGraph(ui.tab1);
	graph2 = new SendLatencyGraph(ui.tab1);
	graph4 = new BandwidthGraph(ui.tab1);

	ui.tab1layout->addWidget(graph1->plot);
	ui.tab2layout->addWidget(graph2->plot);
	ui.tab4layout->addWidget(graph4->plot);

	graph1->setmaxrank(0);
	graph2->setmaxrank(0);

	ui.tabWidget->setCurrentIndex(0);

	resize(800, 480);

    connect(&timer, SIGNAL(timeout()), this, SLOT(getstat()));	

	timer.setInterval(1000);
	timer.start();
}


mpistat::~mpistat()
{
	WSACleanup();
}


Curve::Curve(const int r, const QString& title) 
	: QwtPlotHistogram(title) 
	, rank(r) {
}

   
void Curve::setColor(const QColor &color) {
    QColor c = color;
    c.setAlpha(75);

    setPen(c);
    setBrush(c);
}


HistGraph::HistGraph(QWidget* parent, const int idx)
	: plot(new QwtPlot(parent)) 
	, maxrank(-1) 
	, _idx(idx) {

	plot->canvas()->setFrameShape(QFrame::NoFrame);

    QwtLegend* legend = new QwtLegend;
    legend->setItemMode(QwtLegend::CheckableItem);
    plot->insertLegend(legend, QwtPlot::RightLegend);

	QwtPlotGrid* grid = new QwtPlotGrid;
	grid->enableX(false);
	grid->attach(plot);

    connect(plot, SIGNAL(legendChecked(QwtPlotItem*, bool)),
        SLOT(showCurve(QwtPlotItem*, bool)));

	_color[0] = Qt::red;
	_color[1] = Qt::green;
	_color[2] = Qt::blue;
	_color[3] = Qt::cyan;
	_color[4] = Qt::magenta;
	_color[5] = Qt::darkGray;
	_color[6] = Qt::black;
	_color[7] = Qt::yellow;
}


void HistGraph::showCurve(QwtPlotItem* item, bool on) {

	Curve* curve = (Curve*)item;

    item->setVisible(on);
    QWidget* w = plot->legend()->find(item);

	if ( w && w->inherits("QwtLegendItem") ) {
        ((QwtLegendItem *)w)->setChecked(on);
	}

	curve->showing = on;

	if( _idx == 3 && on ) {

		for( int i = 0; i < curves.size(); i++ ) {
			Curve* c = curves[i];

			if( (c == curve) || 
				(c == curves[curve_rt] && curve == curves[curve_mpi]) ||
				(c == curves[curve_mpi] && curve == curves[curve_rt]) ) {
				c = 0;
			}

			if( c ) {
				c->showing = false;
				c->setVisible(false);

				QWidget* w = plot->legend()->find(c);
				if ( w && w->inherits("QwtLegendItem") ) {
					((QwtLegendItem *)w)->setChecked(false);
				}
			}
		}
	}
    plot->replot();
}


void HistGraph::setmaxrank(int maxrank) {

	int oldsize = curves.size();

	if( maxrank + 1 > oldsize ) {
		curves.resize(maxrank + 1);
		for( int rank = oldsize; rank < curves.size(); rank++ ) {
			Curve* curve = new Curve(rank, QString("rank ") + QString().setNum(rank));
			curve->setColor(_color[rank % 8]);
			curve->attach(plot);
			curves[rank] = curve;
			showCurve(curve, rank == 0);		
		}
	}
	else 
	if( maxrank + 1 < oldsize ) {
		for( int i = curves.size() - 1; i < oldsize; i++ ) {
			if( curves[i] ) {
				curves[i]->detach();
				delete curves[i];
				curves[i] = 0;
			}
		}
		curves.resize(maxrank + 1);
	}
	this->maxrank = maxrank;
}


SizeHistGraph::SizeHistGraph(QWidget* parent)
	: HistGraph(parent, 0) {

    plot->setAxisScale(QwtPlot::xBottom, 16, 65536);
	plot->setAxisScaleEngine(QwtPlot::xBottom, new Log2ScaleEngine());

	QwtText xtitle("Message Size [Bytes]");
	plot->setAxisTitle(QwtPlot::xBottom, xtitle);

	QwtText ytitle("Count");
	plot->setAxisTitle(QwtPlot::yLeft, ytitle);
}


void SizeHistGraph::showstat(const struct msgstats_t& statmsg) {

	int rank = statmsg.rank;
	if( rank < 0 ) {
		return;
	}

	Curve* curve = curves[rank];

	int n = 0;
	for( int i = 0; i < 16; i++ ) {
		if( statmsg.dist[i] > 0 ) {
			++n;
		}
	}

	QVector<QwtIntervalSample> samples(n);                 
	int k = 0;
	for( int i = 3; i < 16; i++ ) {

		double y = 0;

		y = statmsg.dist[i];

		if( y > 0 ) {
			double w0 = (1 << i);
			double w1 = (1 << (i + 1));
			double w = (w0 + w1) / 2;

			if( i == 3 ) {
				w = 16;
			}

			double d = w / 32;

			double x0 = w * 8 / 10 + d;
			double x1 = w * 12 / 10 + d;

			QwtInterval interval(x0, x1);                     
			samples[k++] = QwtIntervalSample(y, interval);                 
		}
	}                 
	curve->setSamples(samples);                 

    plot->replot();
}


void mpistat::srvconnect() {

	//initialize the winsock 2.2
	WSAData wsadata;
	if( WSAStartup(MAKEWORD(2,2), &wsadata) ) { 
		printf("Failed to Startup WSA");
		return; 
	};

	//try to resolve the IP from hostname
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo* addrs;
	char localBuffer[1024];
	if( getaddrinfo(serverName, serverPort, &hints, &addrs) ) {
		printf("failed to resolve ip from hostname %s\n", getLastErrorMessage(localBuffer, 1024, WSAGetLastError()));
		return;
	}

	struct addrinfo* paddr = addrs;
	while( paddr ){
		sprintf(localBuffer, "ip address is %s\n", inet_ntoa(((struct sockaddr_in*)paddr->ai_addr)->sin_addr));
		paddr = paddr->ai_next;
	}

	//create client socket
	toServer = socket(AF_INET, SOCK_STREAM, 0);
	if( ::connect(toServer, addrs->ai_addr, sizeof(*(addrs->ai_addr))) == SOCKET_ERROR ) {
		printf("Failed to Connect, reason %s\n", getLastErrorMessage(localBuffer, 1024, WSAGetLastError()));
		return;
	}
   
    struct sockaddr myaddr;
    int namelength = sizeof(myaddr);
    memset(&myaddr, 0, namelength);
    getsockname(toServer, &myaddr, &namelength);
    printf("Mine=%s\n\n", getSockAddrAsString(localBuffer, 1024, &myaddr));
 
	struct sockaddr peeraddr;
	namelength = sizeof(peeraddr);
	memset(&peeraddr, 0, namelength);
	getpeername(toServer, &peeraddr, &namelength);
	printf("Peer=%s\n\n", getSockAddrAsString(localBuffer, 1024, &peeraddr));

	setWindowTitle(QString("connected to ") + QString(serverName) + QString(":") + QString(serverPort));

	connected = true;	
}
 
void mpistat::getstat() {

	if( !connected ) {
		setWindowTitle(QString("not connected"));
		srvconnect();
	}

	int rank = 0;
	HistGraph* graph;

	if( ui.tabWidget->currentIndex() == 0 ) {
		graph = graph1;
	}
	else 
	if( ui.tabWidget->currentIndex() == 1 ) {
		graph = graph2;
	}
	else 
	if( ui.tabWidget->currentIndex() == 3 ) {
		graph = graph4;
	}
	else {
		return;
	}

	if( ui.tabWidget->currentIndex() <= 2 ) {
		vector<Curve*>::const_iterator it = graph->curves.begin();

		while( (init && rank == 0) || (it != graph->curves.end()) ) {

			Curve* curve = *it;

			if( (init && rank == 0) || (curve->showing) )	{

				if( getstat(rank) ) {

					if( statmsg.maxrank < 0 ) {
						return;
					}

					if( statmsg.maxrank != graph->maxrank ) {
						graph->setmaxrank(statmsg.maxrank);
						return;
					}

					if( statmsg.rank == rank ) {
						graph->showstat(statmsg);
					}
				}
			}

			++rank;
			++it;
		}
		init = false;
	}
	else {
		int maxrank = 99;
		rank = 0;
		while( rank <= maxrank ) {

			if( getstat(rank) ) {

				maxrank = statmsg.maxrank;

				if( statmsg.maxrank != graph->maxrank ) {
					graph->setmaxrank(statmsg.maxrank);
					return;
				}

				graph->showstat(statmsg);						
			}
			++rank;
		}
	}
}


bool mpistat::getstat(int rank) {

	statmsg.rank = rank;

	//send request
	if( ::send(toServer, (char*)&rank, 4, 0) == SOCKET_ERROR ){
		connected = false;
		return false;
	}
	  //receive response
	int rc = ::recv(toServer, (char*)&statmsg, sizeof(statmsg), 0);
	if( (rc == SOCKET_ERROR) || (rc <= 0) ) {
		connected = false;
		return false;
	}

	return true;
}




char* getLastErrorMessage(char* buffer, DWORD size, DWORD errorcode) {

	memset(buffer, 0, size);

	if( FormatMessage(
         FORMAT_MESSAGE_FROM_SYSTEM |
         FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL,
         errorcode,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (LPWSTR)buffer,
         size, NULL) == 0 ) {
   
		//failed in format message, let’s just do error code then
		sprintf(buffer, "Error code is %d", errorcode);
	}
	return buffer;
}

char* getSockAddrAsString(char* buffer, DWORD size, struct sockaddr* saddr) {

	memset(buffer, 0, size);

	if( saddr != 0 ) {
	switch( saddr->sa_family ){
    case AF_INET:
		sprintf(buffer, "AF_INET, Port=%d, IP=%s",
		//note, this place really need to convert network order to host order
		ntohs(((struct sockaddr_in*)saddr)->sin_port),
		inet_ntoa(((struct sockaddr_in*)saddr)->sin_addr));
		break;
	   }
  }
  return buffer;
}


SendLatencyGraph::SendLatencyGraph(QWidget* parent)
	: HistGraph(parent, 1) {

    plot->setAxisScale(QwtPlot::xBottom, 10, 100000);
	plot->setAxisScaleEngine(QwtPlot::xBottom, new Log2ScaleEngine());

	QwtText xtitle("Message Size [Bytes]");
	plot->setAxisTitle(QwtPlot::xBottom, xtitle);

	QwtText ytitle("Latency [us]");
	plot->setAxisTitle(QwtPlot::yLeft, ytitle);
}


void SendLatencyGraph::showstat(const struct msgstats_t& statmsg) {

	int rank = statmsg.rank;
	if( rank < 0 ) {
		return;
	}

	Curve* curve = curves[rank];

	int n = 0;
	for( int i = 0; i < 16; i++ ) {
		if( statmsg.sticks[i] > 0 ) {
			++n;
		}
	}

	QVector<QwtIntervalSample> samples(n);                 
	int k = 0;
	for( int i = 3; i < 16; i++ ) {

		double y = 0;

		y = (double)statmsg.sticks[i];

		if( (y > 0) && (statmsg.dist[i] > 0) && (statmsg.cpufreq > 0) ) {

			y = ((double)y * 1e6) / (statmsg.dist[i] * statmsg.cpufreq);

			double w0 = (1 << i);
			double w1 = (1 << (i + 1));
			double w = (w0 + w1) / 2;

			if( i == 3 ) {
				w = 16;
			}

			double d = w / 32;

			double x0 = w * 8 / 10 + d;
			double x1 = w * 12 / 10 + d;

			QwtInterval interval(x0, x1);                     
			samples[k++] = QwtIntervalSample(y, interval);                 
		}
	}                 
	curve->setSamples(samples);                 

    plot->replot();
}


BandwidthGraph::BandwidthGraph(QWidget* parent)
	: HistGraph(parent, 3) {

    plot->setAxisScale(QwtPlot::xBottom, 0, 7);
    plot->setAxisMaxMajor(QwtPlot::xBottom, 8);
    plot->setAxisMaxMinor(QwtPlot::xBottom, 0);

	QwtText xtitle("MPI rank");
	plot->setAxisTitle(QwtPlot::xBottom, xtitle);

	QwtText ytitle("Total bytes send [MBytes]");
	plot->setAxisTitle(QwtPlot::yLeft, ytitle);

	Curve* curve = new Curve(0, QString("Runtime"));
	curve->setColor(Qt::blue);
	curve->attach(plot);
	curves.push_back(curve);
	showCurve(curve, true);		

	curve = new Curve(0, QString("MPI time"));
	curve->setColor(Qt::red);
	curve->attach(plot);
	curves.push_back(curve);
	showCurve(curve, false);		

	curve = new Curve(0, QString("MBytes send"));
	curve->setColor(Qt::cyan);
	curve->attach(plot);
	curves.push_back(curve);
	showCurve(curve, false);		

	curve = new Curve(0, QString("MPI Send bandwidth"));
	curve->setColor(Qt::green);
	curve->attach(plot);
	curves.push_back(curve);
	showCurve(curve, false);		

	curve = new Curve(0, QString("MPI Receive bandwidth"));
	curve->setColor(Qt::magenta);
	curve->attach(plot);
	curves.push_back(curve);
	showCurve(curve, false);		

	curve = new Curve(0, QString("Memcpy bandwidth"));
	curve->setColor(Qt::darkGray);
	curve->attach(plot);
	curves.push_back(curve);
	showCurve(curve, false);		
}


void BandwidthGraph::setmaxrank(int maxrank) {

	int oldsize = data.size();

	if( maxrank + 1 > oldsize ) {
		data.resize(maxrank + 1);
		for( int rank = oldsize; rank < data.size(); rank++ ) {
			data[rank].bytessend = 0;
			data[rank].rank = rank;
		}
	    plot->setAxisScale(QwtPlot::xBottom, 0, maxrank + 0.5);
	    plot->setAxisMaxMajor(QwtPlot::xBottom, maxrank);
		plot->plotLayout()->setCanvasMargin(60, QwtPlot::yLeft);
	}
	else 
	if( maxrank + 1 < oldsize ) {
		for( int i = data.size() - 1; i < oldsize; i++ ) {
		}
		data.resize(maxrank + 1);

	    plot->setAxisScale(QwtPlot::xBottom, 0, maxrank + 0.5);
	    plot->setAxisMaxMajor(QwtPlot::xBottom, maxrank);
		plot->plotLayout()->setCanvasMargin(60, QwtPlot::yLeft);
	}
	this->maxrank = maxrank;
}


double sticks(const struct msgstats_t& stat) {

	double ticks = 0;
	for( int j = 0; j < 16; j++ ) {
		ticks += stat.sticks[j];
	}
	return ticks;
}


double rticks(const struct msgstats_t& stat) {

	double ticks = 0;
	for( int j = 0; j < 16; j++ ) {
		ticks += stat.rticks[j];
	}
	return ticks;
}


void BandwidthGraph::showstat(const struct msgstats_t& statmsg) {

	int rank = statmsg.rank;
	if( rank < 0 ) {
		return;
	}

	data[statmsg.rank] = statmsg;

	int n = statmsg.maxrank + 1;
	QVector<QwtIntervalSample> samples(n);                 

	if( curves[curve_mbsend]->showing ) {
		QwtText ytitle("Total bytes send [MBytes]");
		plot->setAxisTitle(QwtPlot::yLeft, ytitle);

		for( int i = 0; i < n; i++ ) {
			const struct msgstats_t& stat = data[i];
			double y = (int)(stat.bytessend / (1014 * 1000));
			QwtInterval interval(i - 0.2, i + 0.2);                     
			samples[i] = QwtIntervalSample(y, interval);                 
		}
		curves[curve_mbsend]->setSamples(samples);                 
	}
	else 
	if( curves[curve_rt]->showing || curves[curve_mpi]->showing ) {

		QString ytitle;

		if( curves[curve_rt]->showing ) {
			ytitle += QString("Runtime");
		}
		if( curves[curve_mpi]->showing ) {
			if( curves[curve_rt]->showing ) {
				ytitle += QString(" / ");
			}
			ytitle += QString("MPI-time");
		}
		ytitle += QString(" [sec]");
		plot->setAxisTitle(QwtPlot::yLeft, ytitle);

		if( curves[curve_rt]->showing ) {
			for( int i = 0; i < n; i++ ) {
				const struct msgstats_t& stat = data[i];
				double rt = stat.timesample - stat.timestart;
				QwtInterval interval(i - 0.2, i + 0.2);                     
				samples[i] = QwtIntervalSample(rt, interval);                 
			}
			curves[curve_rt]->setSamples(samples);                 
		}

		if( curves[curve_mpi]->showing ) {
			for( int i = 0; i < n; i++ ) {

				struct msgstats_t& stat = data[i];

				double ticks = sticks(stat);
				ticks += stat.pticks;

				ticks /= stat.cpufreq;
				QwtInterval interval(i - 0.2, i + 0.2);                     
				samples[i] = QwtIntervalSample(ticks, interval);                 
			}
			curves[curve_mpi]->setSamples(samples);                 
		}
	}
	else
	if( curves[curve_sbw]->showing ) {
		QwtText ytitle("MPI Send bandwidth [MBytes/s]");
		plot->setAxisTitle(QwtPlot::yLeft, ytitle);

		for( int i = 0; i < n; i++ ) {
			struct msgstats_t& stat = data[i];
			double ticks = sticks(stat);

			double y = (double)stat.bytessend / (1014.0 * 1024.0) / (ticks / stat.cpufreq);
			QwtInterval interval(i - 0.2, i + 0.2);                     
			samples[i] = QwtIntervalSample(y, interval);                 
		}
		curves[curve_sbw]->setSamples(samples);                 
	}
	else
	if( curves[curve_rbw]->showing ) {
		QwtText ytitle("MPI Receive bandwidth [MBytes/s]");
		plot->setAxisTitle(QwtPlot::yLeft, ytitle);

		for( int i = 0; i < n; i++ ) {
			struct msgstats_t& stat = data[i];
			double ticks = rticks(stat);

			double y = (double)stat.bytessend / (1014.0 * 1000.0) / (ticks / stat.cpufreq);
			QwtInterval interval(i - 0.2, i + 0.2);                     
			samples[i] = QwtIntervalSample(y, interval);                 
		}
		curves[curve_rbw]->setSamples(samples);                 
	}
	else
	if( curves[curve_cpy]->showing ) {
		QwtText ytitle("Memcpy bandwidth [MBytes/s]");
		plot->setAxisTitle(QwtPlot::yLeft, ytitle);

		for( int i = 0; i < n; i++ ) {
			struct msgstats_t& stat = data[i];
			double ticks = stat.mticks;

			double y = (double)stat.bytessend / (1014.0 * 1000.0) / (ticks / stat.cpufreq);
			QwtInterval interval(i - 0.2, i + 0.2);                     
			samples[i] = QwtIntervalSample(y, interval);                 
		}
		curves[curve_cpy]->setSamples(samples);                 
	}


    plot->replot();
}


