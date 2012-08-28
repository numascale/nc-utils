#ifndef NumaChipStats_H
#define NumaChipStats_H

#include <QtGui/QMainWindow>
#include <QtGui/QPaintEvent>
#include <QTimer>
#include "ui_NumaChipStats.h"
#include <vector>
#include <string>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>

#ifdef OS_IS_WINDOWS
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_item.h>

#ifndef OS_IS_WINDOWS
typedef int SOCKET;
typedef unsigned long DWORD;
#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define closesocket close
#endif

using namespace std;
class CacheHistGraph;
class CacheGraph;
class TransactionHist;
class TransGraph;
class ProbeHist;

typedef int int32_t;
typedef unsigned int uint32_t;
#ifdef OS_IS_WINDOWS
typedef unsigned long long uint64_t;
#endif

struct cachestats_t {
    uint64_t hit; //counter_0 - Select = 1, REM/HReq value 6 - HT-Request with ctag miss
    uint64_t miss; //counter_1 - Select = 1, REM/HReq value 5 - HT-Request with ctag hit
    //From totmiss and tothit we can calculate avg hit/miss.  
    uint64_t tothit; //counter_1 - Select = 1, REM/HReq value 5 - HT-Request with ctag hit
    uint64_t totmiss; //counter_0 - Select = 1, REM/HReq value 6 - HT-Request with ctag miss
    uint64_t cave_in; //counter_2 - Select = 7, cHT-Cave value 0 - Incoming non-posted HT-Request
    uint64_t cave_out; //counter_3 - Select = 7, cHT-Cave value 4 - Outgoing non-posted HT-Request
    uint64_t tot_cave_in; //counter_4 - Select = 7, cHT-Cave value 0 - Incoming non-posted HT-Request
    uint64_t tot_cave_out; //counter_5 - Select = 7, cHT-Cave value 4 - Outgoing non-posted HT-Request
    uint64_t tot_probe_in; //counter_6 - Select = 7, cHT-Cave value 3 - Incoming probe HT-Request
    uint64_t tot_probe_out; //counter_7 - Select = 7, cHT-Cave value 7 - Outgoing probe HT-Request
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

class NumaChipStats : public QMainWindow
{
    Q_OBJECT

public:
    NumaChipStats(const string& strCacheAddr, bool simulate, int simulate_nodes);
    ~NumaChipStats();

private:
    Ui::NumaChipStatsClass ui;
    const string cacheAddr;
    SOCKET cacheSocket;
    int m_num_chips;
    bool cacheConnected;
    bool m_freeze;
    bool m_deselected;
    bool m_simulate;
    int m_simulate_nodes;
    int m_spinbox;
    int m_spinbox2;
    

    CacheHistGraph* graph1;
    TransactionHist* graph2;
    CacheGraph* graph5;
    TransGraph* graph3;
    ProbeHist* graph4;
    QTimer timer;

    struct cachestats_t *m_cstat;
    bool init;

    void srvconnect(const string& addr, SOCKET& toServer, bool& connected);
    void showConnectionStatus();
    void getcache();
    

    private slots:
        void getinfo();
        void handleDeselectButton();
        void handleButton();
        void handleBox(int newvalue);
        void handleBox2(int newvalue);
};

class PerfGraph : public QWidget {
    Q_OBJECT   
public:
    PerfGraph(QWidget* parent = 0);

    virtual void showstat(const struct cachestats_t* statmsg) = 0;
    QwtPlot* plot;
    //vector<QwtPlotHistogram*> curves;
    int get_num_chips();
    void set_num_chips(int num);
    void set_range(int min, int max);
    
    public slots:
        void showCurve(QwtPlotItem*, bool on);

protected:
    int p_num_chips;
    int p_range_min;
    int p_range_max;
};
class CacheGraph : public PerfGraph {
    Q_OBJECT   
public:
    CacheGraph(QWidget* parent = 0);
    vector<QwtPlotCurve*> curves;
    void addCurves();
    void showstat(const struct cachestats_t* statmsg);
    double hitrate (unsigned long long hit, unsigned long long miss);    
    void deselectAllLegends(bool turn_off);
private:
    static const int TIME_LENGTH = 250;
    static const int MAX_HITRATE = 100;

    double **m_hitrates;
    double m_timestep[TIME_LENGTH];
    vector <uint64_t> m_transactions[TIME_LENGTH];	
    unsigned int m_counter;
    
};

class TransGraph : public PerfGraph {
    Q_OBJECT   
public:
    TransGraph(QWidget* parent = 0);
    vector<QwtPlotCurve*> curves;
    void addCurves();
    void showstat(const struct cachestats_t* statmsg);
    void deselectAllLegends(bool turn_off);
private:
    static const int TIME_LENGTH = 250;
    static const int MAX_HITRATE = 100;

    double m_timestep[TIME_LENGTH];
    double **m_trans, **m_trans2;
    unsigned int m_counter;
    
};
class PerfHistGraph : public PerfGraph {
    Q_OBJECT   
public:
    PerfHistGraph(QWidget* parent = 0);

    virtual void showstat(const struct cachestats_t* statmsg) = 0;    
    vector<QwtPlotHistogram*> curves;    
    //virtual void addCurves();
};

class CacheHistGraph : public PerfHistGraph {
    Q_OBJECT   
public:
    CacheHistGraph(QWidget* parent = 0);

    virtual void showstat(const struct cachestats_t* statmsg);
    double hitrate (unsigned long long hit, unsigned long long miss);    
    void addCurves();
    void deselectAllLegends(bool turn_off);

private:
    static const int MAX_HITRATE = 100;
    vector <double> m_hitrate;
    vector <uint64_t> m_transactions;    
};


class TransactionHist : public PerfHistGraph {
    Q_OBJECT   
public:
    TransactionHist(QWidget* parent = 0);
    void deselectAllLegends(bool turn_off);
    void showstat(const struct cachestats_t* statmsg);        
    void addCurves();    

private:
    vector <uint64_t> m_transactions;
    vector <uint64_t> m_transactions2;
};
class ProbeHist : public PerfHistGraph {
    Q_OBJECT   
public:
    ProbeHist(QWidget* parent = 0);
    void deselectAllLegends(bool turn_off);
    void showstat(const struct cachestats_t* statmsg);        
    void addCurves();    

private:
    vector <uint64_t> m_transactions;
    vector <uint64_t> m_transactions2;
};
#endif // NumaChipStats_H
