#include "log2scale.hpp"

#include <qwt_scale_engine.h>
#include <qwt_math.h>
#include <qwt_scale_map.h>
#include <qalgorithms.h>
#include <qmath.h>
#include <math.h>


/*!
  Return a transformation, for logarithmic (base 2) scales
*/
QwtScaleTransformation* Log2ScaleEngine::transformation() const {
    return new QwtScaleTransformation(QwtScaleTransformation::Log10);
}

/*!
    Align and divide an interval

   \param maxNumSteps Max. number of steps
   \param x1 First limit of the interval (In/Out)
   \param x2 Second limit of the interval (In/Out)
   \param stepSize Step size (Out)

   \sa QwtScaleEngine::setAttribute()
*/
void Log2ScaleEngine::autoScale(int maxNumSteps,
                                double& x1, double& x2, double& stepSize) const {
    if( x1 > x2 )
        qSwap(x1, x2);

    QwtInterval interval( x1 / qPow( 2.0, lowerMargin() ),
        x2 * qPow( 2.0, upperMargin() ) );

    if ( interval.maxValue() / interval.minValue() < 2.0 )
    {
        // scale width is less than one decade -> build linear scale

        QwtLinearScaleEngine linearScaler;
        linearScaler.setAttributes( attributes() );
        linearScaler.setReference( reference() );
        linearScaler.setMargins( lowerMargin(), upperMargin() );

        linearScaler.autoScale( maxNumSteps, x1, x2, stepSize );
        stepSize = 1.0; //::log2( stepSize );

        return;
    }

    double logRef = 1.0;
    if ( reference() > LOG_MIN / 2 )
        logRef = qMin( reference(), LOG_MAX / 2 );

    if ( testAttribute( QwtScaleEngine::Symmetric ) )
    {
        const double delta = qMax( interval.maxValue() / logRef,
            logRef / interval.minValue() );
        interval.setInterval( logRef / delta, logRef * delta );
    }

    if ( testAttribute( QwtScaleEngine::IncludeReference ) )
        interval = interval.extend( logRef );

    interval = interval.limited( LOG_MIN, LOG_MAX );

    if ( interval.width() == 0.0 )
        interval = buildInterval( interval.minValue() );

    stepSize = divideInterval( log2( interval ).width(), qMax( maxNumSteps, 1 ) );
    if ( stepSize < 1.0 )
        stepSize = 1.0;

    if ( !testAttribute( QwtScaleEngine::Floating ) )
        interval = align( interval, stepSize );

    x1 = interval.minValue();
    x2 = interval.maxValue();

    if ( testAttribute( QwtScaleEngine::Inverted ) )
    {
        qSwap( x1, x2 );
        stepSize = -stepSize;
    }
}

/*!
   \brief Calculate a scale division

   \param x1 First interval limit
   \param x2 Second interval limit
   \param maxMajSteps Maximum for the number of major steps
   \param maxMinSteps Maximum number of minor steps
   \param stepSize Step size. If stepSize == 0, the scaleEngine
                   calculates one.

   \sa QwtScaleEngine::stepSize(), Log2ScaleEngine::subDivide()
*/
QwtScaleDiv Log2ScaleEngine::divideScale( double x1, double x2,
    int maxMajSteps, int maxMinSteps, double stepSize ) const
{
    QwtInterval interval = QwtInterval( x1, x2 ).normalized();
    interval = interval.limited( LOG_MIN, LOG_MAX );

    if ( interval.width() <= 0 )
        return QwtScaleDiv();

    if ( interval.maxValue() / interval.minValue() < 2.0 )
    {
        // scale width is less than one decade -> build linear scale

        QwtLinearScaleEngine linearScaler;
        linearScaler.setAttributes( attributes() );
        linearScaler.setReference( reference() );
        linearScaler.setMargins( lowerMargin(), upperMargin() );

        if ( stepSize != 0.0 )
            stepSize = qPow( 2.0, stepSize );

        return linearScaler.divideScale( x1, x2,
            maxMajSteps, 0, stepSize );
    }

    stepSize = qAbs( stepSize );
    if ( stepSize == 0.0 )
    {
        if ( maxMajSteps < 1 )
            maxMajSteps = 1;

        stepSize = divideInterval( log2( interval ).width(), maxMajSteps );
        if ( stepSize < 1.0 )
            stepSize = 1.0; // major step must be >= 1 decade
    }

    QwtScaleDiv scaleDiv;
    if ( stepSize != 0.0 )
    {
        QList<double> ticks[QwtScaleDiv::NTickTypes];
        buildTicks( interval, stepSize, 0, ticks );

        scaleDiv = QwtScaleDiv( interval, ticks );
    }

    if ( x1 > x2 )
        scaleDiv.invert();

    return scaleDiv;
}

/*!
   \brief Calculate ticks for an interval

   \param interval Interval
   \param maxMinSteps Maximum number of minor steps
   \param stepSize Step size
   \param ticks Arrays to be filled with the calculated ticks

   \sa buildMajorTicks(), buildMinorTicks
*/
void Log2ScaleEngine::buildTicks(
    const QwtInterval& interval, double stepSize, int maxMinSteps,
    QList<double> ticks[QwtScaleDiv::NTickTypes] ) const
{
    const QwtInterval boundingInterval = align( interval, stepSize );

    ticks[QwtScaleDiv::MajorTick] =
        buildMajorTicks( boundingInterval, stepSize );

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
        ticks[i] = strip( ticks[i], interval );
}

/*!
   \brief Calculate major ticks for an interval

   \param interval Interval
   \param stepSize Step size

   \return Calculated ticks
*/
QList<double> Log2ScaleEngine::buildMajorTicks(
    const QwtInterval &interval, double stepSize ) const
{
    double width = log2( interval ).width();

    int numTicks = qRound( width / stepSize ) + 1;
    if ( numTicks > 10000 )
        numTicks = 10000;

    const double lxmin = log( interval.minValue() ) / log(2.0);
    const double lxmax = log( interval.maxValue() ) / log(2.0);
    const double lstep = ( lxmax - lxmin ) / double( numTicks - 1 );

    QList<double> ticks;

    ticks += interval.minValue();

	double t0 = interval.minValue();
	for( int i = 1; i < numTicks - 1; i++ ) {

		for( int j = 0; j < lstep; j++ ) {
			t0 = t0 * 2.0;
		}

        ticks += t0;
	}

    ticks += interval.maxValue();

    return ticks;
}

/*!
   \brief Calculate minor/medium ticks for major ticks

   \param majorTicks Major ticks
   \param maxMinSteps Maximum number of minor steps
   \param stepSize Step size
*/
QList<double> Log2ScaleEngine::buildMinorTicks(
    const QList<double> &majorTicks,
    int maxMinSteps, double stepSize ) const {
    return QList<double>();
}

/*!
  \brief Align an interval to a step size

  The limits of an interval are aligned that both are integer
  multiples of the step size.

  \param interval Interval
  \param stepSize Step size

  \return Aligned interval
*/
QwtInterval Log2ScaleEngine::align(
    const QwtInterval &interval, double stepSize ) const
{
    const QwtInterval intv = log2( interval );

    double x1 = QwtScaleArithmetic::floorEps( intv.minValue(), stepSize );
    if ( qwtFuzzyCompare( interval.minValue(), x1, stepSize ) == 0 )
        x1 = interval.minValue();

    double x2 = QwtScaleArithmetic::ceilEps( intv.maxValue(), stepSize );
    if ( qwtFuzzyCompare( interval.maxValue(), x2, stepSize ) == 0 )
        x2 = interval.maxValue();

    return pow2( QwtInterval( x1, x2 ) );
}

/*!
  Return the interval [log2(interval.minValue(), log2(interval.maxValue]
*/

QwtInterval Log2ScaleEngine::log2( const QwtInterval &interval ) const
{
    return QwtInterval( log( interval.minValue() ) / log(2.0),
            log( interval.maxValue() ) / log(2.0) );
}

/*!
  Return the interval [pow2(interval.minValue(), pow2(interval.maxValue]
*/
QwtInterval Log2ScaleEngine::pow2( const QwtInterval &interval ) const
{
    return QwtInterval( qPow( 2.0, interval.minValue() ),
            qPow( 2.0, interval.maxValue() ) );
}
