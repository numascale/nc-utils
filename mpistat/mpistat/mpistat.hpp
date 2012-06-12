#ifndef MPISTAT_H
#define MPISTAT_H

#include <QtGui/QMainWindow>
#include <QtGui/QPaintEvent>
#include <QTimer>
#include "ui_mpistat.h"
#include <vector>
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
class HistGraph;
class SizeHistGraph;
class BandwidthGraph;

typedef int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

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
	uint32_t dist[16];     // message size ditribution
	uint64_t sticks[16];   // send ticks
	uint64_t rticks[16];   // receive ticks
	uint64_t pticks;	   // progress ticks
	uint64_t mticks;       // memcpy ticks
	uint32_t icnt;		   // messages sync send
	uint32_t qcnt;		   // messages queued send
};


class mpistat : public QMainWindow
{
	Q_OBJECT

public:
	mpistat(QWidget *parent = 0, Qt::WFlags flags = 0);
	~mpistat();

private:
	Ui::mpistatClass ui;
	HistGraph* graph1;
	HistGraph* graph2;
	BandwidthGraph* graph4;
	SOCKET toServer;
	QTimer timer;
	struct msgstats_t statmsg;
	bool  connected;
	bool init;

	void srvconnect();
	bool getstat(int rank);

private slots:
	void getstat();
};


class Curve : public QwtPlotHistogram {
	public:
		Curve(const int r, const QString& title);

	    void setColor(const QColor& color);

		const int rank;
		bool showing;
		int ymax;
};


class HistGraph : public QWidget {
	Q_OBJECT   
public:
	enum {curve_rt, curve_mpi, curve_mbsend, curve_sbw, curve_rbw, curve_cpy};

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
