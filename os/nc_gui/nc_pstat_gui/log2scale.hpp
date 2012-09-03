#include <qwt_scale_engine.h>


class Log2ScaleEngine: public QwtScaleEngine {
public:
    virtual void autoScale( int maxSteps,
        double &x1, double &x2, double &stepSize ) const;

    virtual QwtScaleDiv divideScale( double x1, double x2,
        int numMajorSteps, int numMinorSteps,
        double stepSize = 0.0 ) const;

    virtual QwtScaleTransformation *transformation() const;

protected:
    QwtInterval log2( const QwtInterval& ) const;
    QwtInterval pow2( const QwtInterval& ) const;

    QwtInterval align( const QwtInterval&, double stepSize ) const;

    void buildTicks(
        const QwtInterval &, double stepSize, int maxMinSteps,
        QList<double> ticks[QwtScaleDiv::NTickTypes] ) const;

    QList<double> buildMajorTicks(
        const QwtInterval &interval, double stepSize ) const;

    QList<double> buildMinorTicks(
        const QList<double>& majorTicks,
        int maxMinMark, double step ) const;
};
