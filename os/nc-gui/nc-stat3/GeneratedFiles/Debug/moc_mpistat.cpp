/****************************************************************************
** Meta object code from reading C++ file 'mpistat.hpp'
**
** Created: Tue 14. Aug 10:10:15 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../mpistat.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mpistat.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_mpistat[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x08,
      19,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_mpistat[] = {
    "mpistat\0\0getinfo()\0handleButton()\0"
};

void mpistat::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        mpistat *_t = static_cast<mpistat *>(_o);
        switch (_id) {
        case 0: _t->getinfo(); break;
        case 1: _t->handleButton(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData mpistat::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject mpistat::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_mpistat,
      qt_meta_data_mpistat, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &mpistat::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *mpistat::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *mpistat::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_mpistat))
        return static_cast<void*>(const_cast< mpistat*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int mpistat::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_HistGraph[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   11,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_HistGraph[] = {
    "HistGraph\0\0,on\0showCurve(QwtPlotItem*,bool)\0"
};

void HistGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        HistGraph *_t = static_cast<HistGraph *>(_o);
        switch (_id) {
        case 0: _t->showCurve((*reinterpret_cast< QwtPlotItem*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData HistGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject HistGraph::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_HistGraph,
      qt_meta_data_HistGraph, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HistGraph::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HistGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HistGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HistGraph))
        return static_cast<void*>(const_cast< HistGraph*>(this));
    return QWidget::qt_metacast(_clname);
}

int HistGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_CacheGraph[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      16,   12,   11,   11, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CacheGraph[] = {
    "CacheGraph\0\0,on\0showCurve(QwtPlotItem*,bool)\0"
};

void CacheGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CacheGraph *_t = static_cast<CacheGraph *>(_o);
        switch (_id) {
        case 0: _t->showCurve((*reinterpret_cast< QwtPlotItem*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CacheGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CacheGraph::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CacheGraph,
      qt_meta_data_CacheGraph, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CacheGraph::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CacheGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CacheGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CacheGraph))
        return static_cast<void*>(const_cast< CacheGraph*>(this));
    return QWidget::qt_metacast(_clname);
}

int CacheGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_CacheHistGraph[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   16,   15,   15, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CacheHistGraph[] = {
    "CacheHistGraph\0\0,on\0showCurve(QwtPlotItem*,bool)\0"
};

void CacheHistGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CacheHistGraph *_t = static_cast<CacheHistGraph *>(_o);
        switch (_id) {
        case 0: _t->showCurve((*reinterpret_cast< QwtPlotItem*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData CacheHistGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CacheHistGraph::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_CacheHistGraph,
      qt_meta_data_CacheHistGraph, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CacheHistGraph::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CacheHistGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CacheHistGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CacheHistGraph))
        return static_cast<void*>(const_cast< CacheHistGraph*>(this));
    return QWidget::qt_metacast(_clname);
}

int CacheHistGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_SizeHistGraph[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_SizeHistGraph[] = {
    "SizeHistGraph\0"
};

void SizeHistGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData SizeHistGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SizeHistGraph::staticMetaObject = {
    { &HistGraph::staticMetaObject, qt_meta_stringdata_SizeHistGraph,
      qt_meta_data_SizeHistGraph, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SizeHistGraph::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SizeHistGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SizeHistGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SizeHistGraph))
        return static_cast<void*>(const_cast< SizeHistGraph*>(this));
    return HistGraph::qt_metacast(_clname);
}

int SizeHistGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = HistGraph::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_SendLatencyGraph[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_SendLatencyGraph[] = {
    "SendLatencyGraph\0"
};

void SendLatencyGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData SendLatencyGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SendLatencyGraph::staticMetaObject = {
    { &HistGraph::staticMetaObject, qt_meta_stringdata_SendLatencyGraph,
      qt_meta_data_SendLatencyGraph, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SendLatencyGraph::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SendLatencyGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SendLatencyGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SendLatencyGraph))
        return static_cast<void*>(const_cast< SendLatencyGraph*>(this));
    return HistGraph::qt_metacast(_clname);
}

int SendLatencyGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = HistGraph::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_BandwidthGraph[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_BandwidthGraph[] = {
    "BandwidthGraph\0"
};

void BandwidthGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData BandwidthGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject BandwidthGraph::staticMetaObject = {
    { &HistGraph::staticMetaObject, qt_meta_stringdata_BandwidthGraph,
      qt_meta_data_BandwidthGraph, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BandwidthGraph::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BandwidthGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BandwidthGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BandwidthGraph))
        return static_cast<void*>(const_cast< BandwidthGraph*>(this));
    return HistGraph::qt_metacast(_clname);
}

int BandwidthGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = HistGraph::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
