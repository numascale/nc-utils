#include "mpistat.hpp"
#include "log2scale.hpp"
#include <iostream>

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


mpistat::mpistat(const string& strCacheAddr, const string& strMpiAddr)
	: QMainWindow()
	, cacheAddr(strCacheAddr)
	, mpiAddr(strMpiAddr)
	, timer(this)
	, cacheConnected(false)
	, mpiConnected(false)
	, init(true)
{
	ui.setupUi(this);

	graph1 = new CacheHistGraph(ui.tab1);
	graph2 = new SendLatencyGraph(ui.tab1);
	graph4 = new BandwidthGraph(ui.tab1);
	graph5 = new CacheGraph(ui.tab1);
	ui.tab1layout->addWidget(graph1->plot);
	ui.tab2layout->addWidget(graph2->plot);
	ui.tab4layout->addWidget(graph4->plot);
	ui.tab5layout->addWidget(graph5->plot);

    graph2->setmaxrank(0);

	ui.tabWidget->setCurrentIndex(0);

	resize(800, 480);

    

	if( !cacheAddr.empty() ) {
		srvconnect(cacheAddr, cacheSocket, cacheConnected);
        int num=get_num_chips();
        graph1->set_num_chips(num);
        graph5->set_num_chips(num);
	}

	if( !mpiAddr.empty() ) {
		srvconnect(mpiAddr, mpiSocket, mpiConnected);
	}
    connect(&timer, SIGNAL(timeout()), this, SLOT(getinfo()));	
	timer.setInterval(1000);
	timer.start();
}


mpistat::~mpistat()
{
	WSACleanup();
}


void mpistat::getinfo() {
	getcache();
	getstat();
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

CacheGraph::CacheGraph(QWidget* parent)
	: plot(new QwtPlot(parent)) {
    
	m_counter=0;
    m_num_chips=0;
    for (int i=0; i<240;i++) {
		 m_timestep[i] = i;
	}


	plot->canvas()->setFrameShape(QFrame::NoFrame);

	QwtLegend* legend = new QwtLegend;
	legend->setItemMode(QwtLegend::CheckableItem);
	plot->insertLegend(legend, QwtPlot::BottomLegend);
	plot->setAxisScale(QwtPlot::yLeft, 0, 100);
	plot->setAxisScale(QwtPlot::xBottom, 0, 100);

	QwtPlotGrid* grid = new QwtPlotGrid;
	grid->enableX(false);
	grid->attach(plot);

	QwtText xtitle("Time [s]");	
	plot->setAxisTitle(QwtPlot::xBottom, xtitle);
	QwtText ytitle("Cache hitrate [%]");
	plot->setAxisTitle(QwtPlot::yLeft, ytitle);
   
	connect(plot, SIGNAL(legendChecked(QwtPlotItem*, bool)),
        SLOT(showCurve(QwtPlotItem*, bool)));
}
void CacheGraph::addCurves() {
    char str[80];
    QwtPlotCurve* curve;
    for (int i=0; i<(m_num_chips); i++) {
        sprintf(str, "Remote Cache #%d", i); // s now contains the value 52300         
	    curve = new QwtPlotCurve(str);
        if (i==0) curve->setPen(QPen(Qt::black,2));
        else if (i==1) curve->setPen(QPen(Qt::green,2));
        else if (i==2) curve->setPen(QPen(Qt::blue,2));
        else if (i==3) curve->setPen(QPen(Qt::red,2));
        else if (i==4) curve->setPen(QPen(Qt::cyan,2));
        else if (i==5) curve->setPen(QPen(Qt::magenta,2));
        else if (i==6) curve->setPen(QPen(Qt::yellow,2));
        else if (i==7) curve->setPen(QPen(Qt::darkRed,2));
        else if (i==8) curve->setPen(QPen(Qt::darkGreen,2));
        else if (i==9) curve->setPen(QPen(Qt::darkBlue,2));
        else if (i==10) curve->setPen(QPen(Qt::darkCyan,2));
        else if (i==11) curve->setPen(QPen(Qt::darkMagenta,2));
        else if (i==12) curve->setPen(QPen(Qt::darkYellow,2));
        else if (i==13) curve->setPen(QPen(Qt::darkGray,2));
        else if (i==14) curve->setPen(QPen(Qt::lightGray,2));
        else curve->setPen(QPen(Qt::gray,2));
        curve->attach(plot);
	    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
	    showCurve(curve,true);
	    curves.push_back(curve);	
    }
    //We need initialize the array of doubles:
    /*for (int i=0; i<240;i++) {
        m_hitrates[i]=new double [m_num_chips];        
	}*/
	
}

void CacheGraph::showCurve(QwtPlotItem* item, bool on) {

	QwtPlotCurve* curve = (QwtPlotCurve*)item;

	item->setVisible(on);
	QWidget* w = plot->legend()->find(item);

	if ( w && w->inherits("QwtLegendItem") ) {
        ((QwtLegendItem *)w)->setChecked(on);
	}
	
	QwtText title("Numachip Remote Cache HIT (%)");
        plot->setTitle(title);

	plot->replot();
}

double CacheGraph::hitrate (unsigned long long hit, unsigned long long miss) {

  if (hit + miss==0) {
	  return 100;
  } else if (miss==0) {
	  return 100;
  } else if (hit==0) {
	  return 0;
  } else {
	  return ((double)100*hit/(hit + miss));
  }
}
void CacheGraph::set_num_chips(int numachips) {
    m_num_chips=numachips;
    addCurves();
}
int CacheGraph::get_num_chips(void) {
    return m_num_chips;  
}
void CacheGraph::showstat(const struct cachestats_t& statmsg) {

        if (m_counter == 0) plot->setAxisScale(QwtPlot::xBottom, 0, 100);
        else if (m_counter == 60) plot->setAxisScale(QwtPlot::xBottom, 50, 150); 
        else if (m_counter==120) plot->setAxisScale(QwtPlot::xBottom, 100, 200);
        else if (m_counter==180) plot->setAxisScale(QwtPlot::xBottom, 150, 250);  
	
	{
		
		char s[80];
		QString title;
        for (int i=0; i<m_num_chips; i++) {          
            m_hitrates[m_counter][i]=hitrate(statmsg.hit[i],statmsg.miss[i]);
            //m_hitrates[m_counter].push_back(hitrate(statmsg.hit[i],statmsg.miss[i]));
            m_transactions[m_counter].push_back(statmsg.hit[i] + statmsg.miss[i]);
            	    
        	sprintf(s, "Remote Cache #%d transactions %lld", 
                i, 
                m_transactions[m_counter][i]); 
		    title.append(QString(s));		
		    curves[i]->setTitle(title);	    
            title.clear();
            //curves[i]->setRawSamples(m_timestep, _hitrate.hitarray, m_counter);
            //curves[0]->setRawSamples(sample_x, sample_y0, counter);
            curves[i]->setRawSamples(m_timestep, (m_hitrates)[i], m_counter);
            //curves[i]->setSamples((m_timestep)[i], (m_hitrates)[i]);            
        }
	
	}
	
    
	plot->replot();
    m_transactions[m_counter].clear();
    m_counter = m_counter++;
	if (m_counter==240) m_counter=0;

  
}

CacheHistGraph::CacheHistGraph(QWidget* parent)
    : plot(new QwtPlot(parent)) {

        m_num_chips=0;
        plot->canvas()->setFrameShape(QFrame::NoFrame);


        QwtLegend* legend = new QwtLegend;
        legend->setItemMode(QwtLegend::CheckableItem);
        plot->insertLegend(legend, QwtPlot::BottomLegend);
        plot->setAxisScale(QwtPlot::xBottom, 0, m_num_chips);
        plot->setAxisMaxMajor(QwtPlot::xBottom, m_num_chips + 1);
        plot->setAxisMaxMinor(QwtPlot::xBottom, 0);
        plot->setAxisScale(QwtPlot::yLeft, 0, 100);


        QwtPlotGrid* grid = new QwtPlotGrid;
        grid->enableX(false);
        grid->attach(plot);

        QwtText xtitle("Remote Cache #n");	
        plot->setAxisTitle(QwtPlot::xBottom, xtitle);
        QwtText ytitle("Cache hitrate [%]");
        plot->setAxisTitle(QwtPlot::yLeft, ytitle);

        connect(plot, SIGNAL(legendChecked(QwtPlotItem*, bool)),
            SLOT(showCurve(QwtPlotItem*, bool)));

}
void CacheHistGraph::addCurves() {
    char str[80];
    QwtPlotHistogram* curve;
    for (int i=0; i<(m_num_chips*2); i++) {
        QColor blue = Qt::blue, red = Qt::red;

        red.setAlpha(75);
        blue.setAlpha(75);
        if (i<m_num_chips) {
            sprintf(str, "Remote Cache #%d", i); // s now contains the value 52300         
            curve = new QwtPlotHistogram(str);
            curve->setBrush(blue);
            curve->setPen(blue);
            curve->attach(plot);
            curves.push_back(curve);
            showCurve(curve, true);			      
        } else {
            sprintf(str, "Average Remote Cache #%d", i-m_num_chips); // s now contains the value 52300         
            curve = new QwtPlotHistogram(str);
            curve->setBrush(red);
            curve->setPen(red);
            curve->attach(plot);
            curves.push_back(curve);
            showCurve(curve, true);		
        }

    }

}

void CacheHistGraph::showCurve(QwtPlotItem* item, bool on) {

    item->setVisible(on);
	  QWidget* w = plot->legend()->find(item);

	  if ( w && w->inherits("QwtLegendItem") ) {
        ((QwtLegendItem *)w)->setChecked(on);
	  }
	
	  QwtText title("Numachip Remote Cache HIT (%)");
      plot->setTitle(title);
	  plot->replot();

}

double CacheHistGraph::hitrate (unsigned long long hit, unsigned long long miss) {
    cout << "hit " << hit << " miss " << miss << " avg " << (double)100*hit/(hit + miss) << endl;
    if (hit + miss==0) {
        return 100;
    } else if (miss==0) {
        return 100;
    } else if (hit==0) {
        return 0;
    } else {
        return ((double)100*hit/(hit + miss));
    }
}
void CacheHistGraph::set_num_chips(int numachips) {
    m_num_chips=numachips;
    addCurves();
    
}
int CacheHistGraph::get_num_chips(void) {
    return m_num_chips;  
}
void CacheHistGraph::showstat(const struct cachestats_t& statmsg) {

    plot->setAxisScale(QwtPlot::xBottom,-1, m_num_chips + 0.5);
    plot->setAxisMaxMajor(QwtPlot::xBottom, m_num_chips);
    plot->plotLayout()->setCanvasMargin(20, QwtPlot::yLeft);

    char s[80];
    QString title;
    QVector<QwtIntervalSample> samples(1);  
    for (int i=0; i<(m_num_chips*2); i++) {

        if (i<m_num_chips) {
            m_hitrate.push_back(hitrate(statmsg.hit[i],statmsg.miss[i]));
            m_transactions.push_back(statmsg.hit[i] + statmsg.miss[i]);
            sprintf(s, "Remote Cache #%d transactions %lld", i,m_transactions[i]); 
            samples[0]=QwtIntervalSample(m_hitrate[i], i-0.2,i+0.2);
            curves[i]->setSamples(samples);
        } else {
            m_hitrate.push_back(hitrate(statmsg.tothit[i-m_num_chips],statmsg.totmiss[i-m_num_chips]));		    
            m_transactions.push_back(statmsg.tothit[i-m_num_chips] + statmsg.totmiss[i-m_num_chips]);
            sprintf(s, "Average Remote Cache #%d transactions %lld",i-m_num_chips, m_transactions[i]); 
            samples[0]=QwtIntervalSample(m_hitrate[i], (i-m_num_chips)-0.2,(i-m_num_chips)+0.2);
            curves[i]->setSamples(samples);
        }

        title.append(QString(s));		
        curves[i]->setTitle(title);
        title.clear();

    }

    plot->replot();    
    m_hitrate.clear();
    m_transactions.clear();
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
				(c == curves[curve_mpi] && curve == curves[curve_rt]) ||			
				
				(c == curves[curve_rt] && curve == curves[curve_blocking]) ||
				(c == curves[curve_blocking] && curve == curves[curve_rt]) ) {
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

	QwtText xtitle("Node cnt [Number]");
	plot->setAxisTitle(QwtPlot::xBottom, xtitle);

	QwtText ytitle("Cache hitrate [%]");
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

void mpistat::srvconnect(const string& addr, SOCKET& toServer, bool& connected) {

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

	char serverName[256];
	char serverPort[256];

	strcpy(serverName, addr.c_str());
	serverPort[0] = 0;

	if( strlen(serverName) > 0 ) {
		char* p = strchr(serverName, ':');
		if( p ) {
			if( strlen(serverName) > (p - serverName) + 1 ) {
				*p = '\0';
				strcpy(serverPort, p + 1);
			}
		}
	}

	if( getaddrinfo(serverName, serverPort, &hints, &addrs) ) {
		printf("failed to resolve ip from hostname %s\n", getLastErrorMessage(localBuffer, 1024, WSAGetLastError()));
		connected = false;	
		showConnectionStatus();
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
		connected = false;	
		showConnectionStatus();
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

	connected = true;	
	showConnectionStatus();
}


void mpistat::showConnectionStatus() {
	QString title;

    if( !cacheAddr.empty() ) {
		if( cacheConnected ) {
			title = QString("connected to ") + QString(cacheAddr.c_str());
		}
		else {
			title = QString("not connected to ") + QString(cacheAddr.c_str());
		}
	}

	if( !mpiAddr.empty() ) {
		if( !cacheAddr.empty() ) {
			title += QString(",  ");
		}

		if( mpiConnected ) {
			title += QString("connected to ") + QString(mpiAddr.c_str());
		}
		else {
			title += QString("not connected to ") + QString(mpiAddr.c_str());
		}
	}

	setWindowTitle(title);
}


void mpistat::getcache() {

	if( !cacheConnected ) {
		return;
	}
    //We need to get information from client on how many numachips there are in the system
    //After getting this information, then we know how many graphs to create. 
    //We need a function returning the number of numachips in the system
    int any = 0;
	struct cachestats_t cstat;
	
	//send request
	if( ::send(cacheSocket, (char*)&any, sizeof(int), 0) == SOCKET_ERROR ){
		cacheConnected = false;
		showConnectionStatus();
		return;
	}
	  //receive response
	int rc = ::recv(cacheSocket, (char*)&cstat, sizeof(cstat), 0);
	if( (rc == SOCKET_ERROR) || (rc <= 0) ) {
		cacheConnected = false;
		showConnectionStatus();
		return;
	}
 
	graph5->showstat(cstat);
    graph1->showstat(cstat);
}

void mpistat::getstat() {

	if( !mpiConnected ) {
		return;
	}

	if( !mpiConnected ) {
		srvconnect(mpiAddr, mpiSocket, mpiConnected);

		if( !mpiConnected ) {
			return;
		}
	}

	int rank = 0;
	HistGraph* graph;

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

	if( !mpiConnected ) {
		return false;
	}

	statmsg.rank = rank;

	//send request
	if( ::send(mpiSocket, (char*)&rank, 4, 0) == SOCKET_ERROR ){
		mpiConnected = false;
		showConnectionStatus();
		return false;
	}
	  //receive response
	int rc = ::recv(mpiSocket, (char*)&statmsg, sizeof(statmsg), 0);
	if( (rc == SOCKET_ERROR) || (rc <= 0) ) {
		mpiConnected = false;
		showConnectionStatus();
		return false;
	}

	return true;
}
int mpistat::get_num_chips() {
    int num_chips=0;
	if( !cacheConnected ) {
		return 0;
	}

	//send request
	if( ::send(cacheSocket, (char*)&num_chips, 4, 0) == SOCKET_ERROR ){
		cacheConnected = false;
		showConnectionStatus();
		return 0;
	}
	  //receive response
	int rc = ::recv(cacheSocket, (char*)&num_chips, sizeof(num_chips), 0);
	if( (rc == SOCKET_ERROR) || (rc <= 0) ) {
		cacheConnected = false;
		showConnectionStatus();
		return 0;
	}

	return num_chips;
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

	curve = new Curve(0, QString("Send blocking"));
	curve->setColor(Qt::red);
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
		plot->plotLayout()->setCanvasMargin(20, QwtPlot::yLeft);
	}
	else 
	if( maxrank + 1 < oldsize ) {
		for( int i = data.size() - 1; i < oldsize; i++ ) {
		}
		data.resize(maxrank + 1);

	    plot->setAxisScale(QwtPlot::xBottom, 0, maxrank + 0.5);
	    plot->setAxisMaxMajor(QwtPlot::xBottom, maxrank);
		plot->plotLayout()->setCanvasMargin(20, QwtPlot::yLeft);
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
	if( curves[curve_rt]->showing || curves[curve_mpi]->showing || curves[curve_blocking]->showing ) {

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
		if( curves[curve_blocking]->showing ) {
			if( curves[curve_blocking]->showing ) {
				ytitle += QString(" / ");
			}
			ytitle += QString("Send blocking-time");
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

		if( curves[curve_blocking]->showing ) {
			for( int i = 0; i < n; i++ ) {
				struct msgstats_t& stat = data[i];
				double y = ((double)stat.icnt + (double)stat.scnt) / (double)stat.cpufreq;
				QwtInterval interval(i - 0.2, i + 0.2);                     
				samples[i] = QwtIntervalSample(y, interval);                 
			}
			curves[curve_blocking]->setSamples(samples);                 
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


