#include <QtGui/QWidget>
#include <vector>

#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_histogram.h>
#include <qwt_plot_item.h>

using namespace std;
class Curve;

class MpiGraph : public QWidget {
	Q_OBJECT   
public:
	MpiGraph(QWidget* parent = 0);

	void showstat(const struct msgstats_t& statmsg);

	QwtPlot* plot;

private slots:
	void showCurve(QwtPlotItem*, bool on);

private:
	vector<Curve*> curves;
	int _ymax;
	QColor _color[8];
};
