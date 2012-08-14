#ifndef MPISTAT_H
#define MPISTAT_H

#include <QtGui/QMainWindow>
#include <QtGui/QPaintEvent>
#include <QTimer>
#include "ui_mpistat.h"
#include <vector>
#include <string>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_item.h>

using namespace std;
class CacheHistGraph;
class CacheGraph;
class HistGraph;
class SizeHistGraph;
class BandwidthGraph;

typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

struct cachestats_t {
    uint64_t hit; //counter_0 - Select = 1, REM/HReq value 6 - HT-Request with ctag miss
    uint64_t miss; //counter_1 - Select = 1, REM/HReq value 5 - HT-Request with ctag hit

    //From totmiss and tothit we can calculate avg hit/miss.  
    uint64_t tothit; //counter_1 - Select = 1, REM/HReq value 5 - HT-Request with ctag hit
    uint64_t totmiss; //counter_0 - Select = 1, REM/HReq value 6 - HT-Request with ctag miss
    uint64_t cave_in; //counter_2 - Select = 7, cHT-Cave value 0 - Incoming non-posted HT-Request
    uint64_t cave_out; //counter_3 - Select = 7, cHT-Cave value 4 - Outgoing non-posted HT-Request
    
    /*
    uint64_t counter_4[4];
    uint64_t counter_5[4];
    uint64_t counter_6[4];
    uint64_t counter_7[4];  
    */
};

struct msgstats_t {
    int32_t  maxrank;
    int32_t  rank;
    int32_t  totmsgs;
    int32_t  active;
    uint64_t bytessend;
    uint64_t timestart;
    uint64_t timesample;
    uint32_t cpuid;
    uint64_t cpufreq;
    uint32_t dist[32];     // message size ditribution
    uint64_t sticks[32];   // send ticks (eager msgs only)
    uint64_t rticks[32];   // receive ticks (eager msgs only)
    uint64_t pticks;	   // progress ticks
    uint64_t mticks;       // memcpy ticks
    uint64_t icnt;		   // ring no space ticks
    uint64_t scnt;		   // sync buf lock failed ticks
};

class mpistat : public QMainWindow
{
    Q_OBJECT

public:
    mpistat(const string& strCacheAddr, const string& strMpiAddr);
    ~mpistat();

private:
    Ui::mpistatClass ui;
    const string cacheAddr;
    const string mpiAddr;    
    SOCKET cacheSocket;
    SOCKET mpiSocket;
    int m_num_chips;
    bool  cacheConnected;
    bool  m_freeze;
    bool  mpiConnected;

    CacheHistGraph* graph1;
    HistGraph* graph2;
    BandwidthGraph* graph4;
    CacheGraph* graph5;

    QTimer timer;
    struct msgstats_t statmsg;
    struct cachestats_t *m_cstat;
    bool init;

    void srvconnect(const string& addr, SOCKET& toServer, bool& connected);
    void showConnectionStatus();
    bool getstat(int rank);
    /*int  get_num_chips();*/

    void getstat();
    void getcache();

    private slots:
        void getinfo();
        void handleButton();
};

class Curve : public QwtPlotHistogram {
public:
    Curve(const int r, const QString& title);
    void setColor(const QColor& color);
    const int rank;
    bool showing;
    int ymax;
};
class CacheGraph : public QWidget {
    Q_OBJECT   
public:

    CacheGraph(QWidget* parent = 0);

    QwtPlot* plot;
    vector<QwtPlotCurve*> curves;
    void addCurves();
    void clear_num_chips();
    void showstat(const struct cachestats_t* statmsg);
    double hitrate (unsigned long long hit, unsigned long long miss);
    int get_num_chips();
    void set_num_chips(int num);

    public slots:
        void showCurve(QwtPlotItem*, bool on);

private:
    static const int TIME_LENGTH = 250;
    static const int MAX_HITRATE = 100;

    double **m_hitrates;
    double m_timestep[TIME_LENGTH];
    vector <uint64_t> m_transactions[TIME_LENGTH];	
    unsigned int m_counter;
    int p_num_chips;

};

class PerfHistGraph : public QWidget {
    Q_OBJECT   
public:
    PerfHistGraph(QWidget* parent = 0);

    virtual void showstat(const struct cachestats_t* statmsg) = 0;
    QwtPlot* plot;
    vector<QwtPlotHistogram*> curves;
    int get_num_chips();
    virtual void addCurves();
 
    public slots:
        void showCurve(QwtPlotItem*, bool on);

protected:
    int p_num_chips;
};

class CacheHistGraph : public PerfHistGraph {
    Q_OBJECT   
public:
    CacheHistGraph(QWidget* parent = 0);

    virtual void showstat(const struct cachestats_t* statmsg);
    double hitrate (unsigned long long hit, unsigned long long miss);
    void set_num_chips(int num);
    void addCurves();
 

private:
    static const int MAX_HITRATE = 100;
    vector <double> m_hitrate;
    vector <uint64_t> m_transactions;    
};

/*
class TransactionHist : public QWidget {
    Q_OBJECT   
public:
    TransactionHist(QWidget* parent = 0);

    void showstat(const struct cachestats_t* statmsg);
    QwtPlot* plot;
    vector<QwtPlotHistogram*> curves;
    int get_num_chips();
    void set_num_chips(int num);    
    void addCurves();
    void clear_num_chips();
    public slots:
        void showCurve(QwtPlotItem*, bool on);

private:
    /*static const int MAX_HITRATE = 100;

    vector <uint64_t> m_transactions;
    vector <uint64_t> m_transactions2

    int p_num_chips;
};
*/
class HistGraph : public QWidget {
    Q_OBJECT   
public:
    enum {curve_rt, curve_mpi, curve_mbsend, curve_sbw, curve_rbw, curve_cpy, curve_blocking};

    HistGraph(QWidget* parent = 0, const int idx = -1);

    virtual void showstat(const struct msgstats_t& statmsg) = 0;
    virtual void setmaxrank(int maxrank);

    QwtPlot* plot;
    vector<Curve*> curves;
    int maxrank;

    public slots:
        void showCurve(QwtPlotItem*, bool on);

protected:
    const int _idx;
    QColor _color[8];
};

class SizeHistGraph : public HistGraph {
    Q_OBJECT   
public:
    SizeHistGraph(QWidget* parent = 0);

    virtual void showstat(const struct msgstats_t& statmsg);
};


class SendLatencyGraph : public HistGraph {
    Q_OBJECT   
public:
    SendLatencyGraph(QWidget* parent = 0);

    virtual void showstat(const struct msgstats_t& statmsg);
};

class BandwidthGraph : public HistGraph {
    Q_OBJECT   
public:
    BandwidthGraph(QWidget* parent = 0);

    virtual void showstat(const struct msgstats_t& statmsg);
    virtual void setmaxrank(int maxrank);

private:
    vector<struct msgstats_t> data;
};


#endif // MPISTAT_H