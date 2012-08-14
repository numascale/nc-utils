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


mpistat::mpistat(const string& strCacheAddr)
    : QMainWindow()
    , cacheAddr(strCacheAddr)
    , timer(this)
    , cacheConnected(false)
    , m_freeze(true)
    , m_num_chips(0)
    , init(true)
{
    ui.setupUi(this);

    graph1 = new CacheHistGraph(ui.tab1);
    //graph2 = new SendLatencyGraph(ui.tab1);
    //graph4 = new BandwidthGraph(ui.tab1);
    graph5 = new CacheGraph(ui.tab1);
    ui.tab1layout->addWidget(graph1->plot);
    //ui.tab2layout->addWidget(graph2->plot);
    //ui.tab4layout->addWidget(graph4->plot);
    ui.tab5layout->addWidget(graph5->plot);
    ui.pushButton->setToolTip("Stop the communication with the Numascale master node deamon temporarily.");
    //graph2->setmaxrank(0);
    ui.tabWidget->setCurrentIndex(1);
    resize(800, 480);
    if( !cacheAddr.empty() ) {
        srvconnect(cacheAddr, cacheSocket, cacheConnected);
    }
    
    connect(&timer, SIGNAL(timeout()), this, SLOT(getinfo()));	
    connect(ui.pushButton, SIGNAL(released()), this, SLOT(handleButton()));
    timer.setInterval(1000);
    timer.start();
}

mpistat::~mpistat()
{
    WSACleanup();
}

void mpistat::getinfo() {

    if (m_freeze) {
        getcache();
        if( !cacheConnected ) {
            if( !cacheAddr.empty() ) {
                srvconnect(cacheAddr, cacheSocket, cacheConnected);
            }           
        }
    }
}
void mpistat::handleButton() {	
    if (m_freeze) {
        ui.pushButton->setText("Continue");
        ui.pushButton->setToolTip("Continue the communication with the Numascale master node deamon again to receive fresh statistics.");
    } else {
        ui.pushButton->setText("Freeze");
        ui.pushButton->setToolTip("Stop the communication with the Numascale master node deamon temporarily.");
    }
    m_freeze=!m_freeze;
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
PerfGraph::PerfGraph(QWidget* parent)
    : plot(new QwtPlot(parent)) {

      p_num_chips=0;
      plot->canvas()->setFrameShape(QFrame::NoFrame);

      QwtLegend* legend = new QwtLegend;
      legend->setItemMode(QwtLegend::CheckableItem);
      plot->insertLegend(legend, QwtPlot::BottomLegend);
    
      QwtPlotGrid* grid = new QwtPlotGrid;
      grid->enableX(false);
      grid->attach(plot);

      connect(plot, SIGNAL(legendChecked(QwtPlotItem*, bool)),
            SLOT(showCurve(QwtPlotItem*, bool)));
}
void PerfGraph::showCurve(QwtPlotItem* item, bool on) {

    item->setVisible(on);
    QWidget* w = plot->legend()->find(item);

    if ( w && w->inherits("QwtLegendItem") ) {
        ((QwtLegendItem *)w)->setChecked(on);
    }        
    plot->replot();
}
int PerfGraph::get_num_chips(void) {
    return p_num_chips;  
}
CacheGraph::CacheGraph(QWidget* parent) {

    m_counter=0;

    for (int i=0; i<TIME_LENGTH;i++) {
        m_timestep[i] = i;
    }

    plot->setAxisScale(QwtPlot::yLeft, 0, MAX_HITRATE);
    plot->setAxisScale(QwtPlot::xBottom, 0, MAX_HITRATE);


    QwtText xtitle("Time [s]");	
    plot->setAxisTitle(QwtPlot::xBottom, xtitle);
    QwtText ytitle("Cache hitrate [%]");
    plot->setAxisTitle(QwtPlot::yLeft, ytitle);
    QwtText title("Numachip Remote Cache HIT (%)");
    plot->setTitle(title);
    plot->replot();
}
void CacheGraph::addCurves() {

    char str[80];
    QwtPlotCurve* curve;
    curves.clear();
    for (int i=0; i<p_num_chips; i++) {
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
    m_hitrates=new double *[p_num_chips]; 
    for (int i=0; i<p_num_chips;i++) {
        m_hitrates[i]=new double[TIME_LENGTH]; 
        for (int j=0; j<TIME_LENGTH; j++) {
            m_hitrates[i][j]=MAX_HITRATE;
        }
    }

}
double CacheGraph::hitrate (unsigned long long hit, 
    unsigned long long miss) {

    if (hit + miss==0) {
        return MAX_HITRATE;
    } else if (miss==0) {
        return MAX_HITRATE;
    } else if (hit==0) {
        return 0;
    } else {
        return ((double)100*hit/(hit + miss));
    }
}
void CacheGraph::set_num_chips(int numachips) {
    p_num_chips=numachips;
    addCurves();
}
void CacheGraph::showstat(const struct cachestats_t* statmsg) {

    if (m_counter == 0) plot->setAxisScale(QwtPlot::xBottom, 0, MAX_HITRATE);
    else if (m_counter == 60) plot->setAxisScale(QwtPlot::xBottom, TIME_LENGTH/5, 3*TIME_LENGTH/5); 
    else if (m_counter==120) plot->setAxisScale(QwtPlot::xBottom, 2*TIME_LENGTH/5, 4*TIME_LENGTH/5);
    else if (m_counter==180) plot->setAxisScale(QwtPlot::xBottom, 3*TIME_LENGTH/5, TIME_LENGTH);  

    char s[80];
    QString title;
    for (int i=0; i<p_num_chips; i++) {          
        m_hitrates[i][m_counter]=hitrate(statmsg[i].hit,statmsg[i].miss);
        m_transactions[m_counter].push_back(statmsg[i].hit + statmsg[i].miss);
        sprintf(s, "Remote Cache #%d transactions %lld", 
            i, 
            m_transactions[m_counter][i]); 
        title.append(QString(s));		
        curves[i]->setTitle(title);	    
        title.clear();   
        curves[i]->setRawSamples(m_timestep, *(m_hitrates + i), m_counter);
        printf("Remote Cache #%d transactions %lld hitrate %.2f m_timestep %.2f m_counter %d\n", 
            i, 
            m_transactions[m_counter][i],m_hitrates[i][m_counter], m_timestep[i], m_counter );
    }

    plot->replot();
    m_transactions[m_counter].clear();
    m_counter = m_counter++;
    if (m_counter==TIME_LENGTH) m_counter=0;
}
PerfHistGraph::PerfHistGraph(QWidget* parent) {
      plot->setAxisScale(QwtPlot::xBottom, 0, p_num_chips);
      plot->setAxisMaxMajor(QwtPlot::xBottom, p_num_chips + 1);
      plot->setAxisMaxMinor(QwtPlot::xBottom, 0);    
}
void PerfHistGraph::addCurves() {
 
    char str[80];
    QwtPlotHistogram* curve;
    curves.clear();
    for (int i=0; i<(p_num_chips*2); i++) {
        QColor blue = Qt::blue, red = Qt::red;

        red.setAlpha(75);
        blue.setAlpha(75);
        if (i<p_num_chips) {
            sprintf(str, "Numachip #%d", i); // s now contains the value 52300         
            curve = new QwtPlotHistogram(str);
            curve->setBrush(blue);
            curve->setPen(blue);
            curve->attach(plot);
            curves.push_back(curve);
            showCurve(curve, true);			      
        } else {
            sprintf(str, "Average Remote Cache #%d", i-p_num_chips); // s now contains the value 52300         
            curve = new QwtPlotHistogram(str);
            curve->setBrush(red);
            curve->setPen(red);
            curve->attach(plot);
            curves.push_back(curve);
            showCurve(curve, true);		
        }
    }
}
CacheHistGraph::CacheHistGraph(QWidget* parent) {

      plot->setAxisScale(QwtPlot::yLeft, 0, MAX_HITRATE);
      QwtText xtitle("Remote Cache #n");	
      plot->setAxisTitle(QwtPlot::xBottom, xtitle);
      
      QwtText ytitle("Cache hitrate [%]");
      plot->setAxisTitle(QwtPlot::yLeft, ytitle);
      QwtText title("Numachip Remote Cache HIT (%)");
      plot->setTitle(title);      
}
void CacheHistGraph::set_num_chips(int numachips) {
    p_num_chips=numachips;
    addCurves();
}
void CacheHistGraph::addCurves() {
 
    char str[80];
    QwtPlotHistogram* curve;
    curves.clear();
    for (int i=0; i<(p_num_chips*2); i++) {
        QColor blue = Qt::blue, red = Qt::red;

        red.setAlpha(75);
        blue.setAlpha(75);
        if (i<p_num_chips) {
            sprintf(str, "Numachip #%d", i); // s now contains the value 52300         
            curve = new QwtPlotHistogram(str);
            curve->setBrush(blue);
            curve->setPen(blue);
            curve->attach(plot);
            curves.push_back(curve);
            showCurve(curve, true);			      
        } else {
            sprintf(str, "Average Remote Cache #%d", i-p_num_chips); // s now contains the value 52300         
            curve = new QwtPlotHistogram(str);
            curve->setBrush(red);
            curve->setPen(red);
            curve->attach(plot);
            curves.push_back(curve);
            showCurve(curve, true);		
        }
    }
}
double CacheHistGraph::hitrate (unsigned long long hit, unsigned long long miss) {
    //cout << "hit " << hit << " miss " << miss << " avg " << (double)100*hit/(hit + miss) << endl;
    if (hit + miss==0) {
        return MAX_HITRATE;
    } else if (miss==0) {
        return MAX_HITRATE;
    } else if (hit==0) {
        return 0;
    } else {
        return ((double)100*hit/(hit + miss));
    }
}
void CacheHistGraph::showstat(const struct cachestats_t* statmsg) {

    plot->setAxisScale(QwtPlot::xBottom,-1, p_num_chips + 0.5);
    plot->setAxisMaxMajor(QwtPlot::xBottom, p_num_chips);
    plot->plotLayout()->setCanvasMargin(20, QwtPlot::yLeft);

    char s[80];
    QString title;
    QVector<QwtIntervalSample> samples(1);  
    for (int i=0; i<(p_num_chips*2); i++) {

        if (i<p_num_chips) {
            m_hitrate.push_back(hitrate(statmsg[i].hit,statmsg[i].miss));
            m_transactions.push_back(statmsg[i].hit + statmsg[i].miss);
            sprintf(s, "Remote Cache #%d transactions %lld", i,m_transactions[i]); 
            samples[0]=QwtIntervalSample(m_hitrate[i], i-0.2,i+0.2);
            curves[i]->setSamples(samples);
        } else {
            m_hitrate.push_back(hitrate(statmsg[i-p_num_chips].tothit,statmsg[i-p_num_chips].totmiss));		    
            m_transactions.push_back(statmsg[i-p_num_chips].tothit + statmsg[i-p_num_chips].totmiss);
            sprintf(s, "Average Remote Cache #%d transactions %lld",i-p_num_chips, m_transactions[i]); 
            samples[0]=QwtIntervalSample(m_hitrate[i], (i-p_num_chips)-0.2,(i-p_num_chips)+0.2);
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


    setWindowTitle(title);
}

void mpistat::getcache() {

    if( !cacheConnected ) {
        return;
    }

    //We need to get information from client on how many numachips there are in the system
    //After getting this information, then we know how many graphs to create. 
    //We need a function returning the number of numachips in the system
    int num_chips = 0;

    //send request
    if( ::send(cacheSocket, (char*)&num_chips, sizeof(int), 0) == SOCKET_ERROR ){
        cacheConnected = false;
        showConnectionStatus();
        return;
    }

    //receive response    
    int rc = ::recv(cacheSocket, (char*)&num_chips, sizeof(num_chips), 0);    
    if( (rc == SOCKET_ERROR) || (rc <= 0) ) {    
        cacheConnected = false;
        showConnectionStatus();
        return;
    }
    
    if (m_num_chips!=num_chips) {
         
        /* We need to clean the old setup in order not to leak memory
         * Better clean up this code.
         */
        if (m_num_chips>0) delete m_cstat;
        
        m_num_chips=num_chips;
        graph1->set_num_chips(m_num_chips);
        graph5->set_num_chips(m_num_chips);

        printf("Master node in the numaconnect single image cluster reports %d numchips.\n", 
            m_num_chips);

        m_cstat = new cachestats_t[m_num_chips];
    }


    //receive response
    rc = ::recv(cacheSocket, (char*)m_cstat, m_num_chips*sizeof(struct cachestats_t), 0);
    if( (rc == SOCKET_ERROR) || (rc <= 0) ) {
        cacheConnected = false;
        showConnectionStatus();
        return;
    }

    graph5->showstat(m_cstat);
    graph1->showstat(m_cstat);
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
