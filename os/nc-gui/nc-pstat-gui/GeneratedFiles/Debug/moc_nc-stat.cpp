/****************************************************************************
** Meta object code from reading C++ file 'nc-stat.hpp'
**
** Created: Tue 28. Aug 16:35:05 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../nc-stat.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'nc-stat.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_NumaChipStats[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      25,   14,   14,   14, 0x08,
      48,   14,   14,   14, 0x08,
      72,   63,   14,   14, 0x08,
      87,   63,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_NumaChipStats[] = {
    "NumaChipStats\0\0getinfo()\0"
    "handleDeselectButton()\0handleButton()\0"
    "newvalue\0handleBox(int)\0handleBox2(int)\0"
};

void NumaChipStats::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        NumaChipStats *_t = static_cast<NumaChipStats *>(_o);
        switch (_id) {
        case 0: _t->getinfo(); break;
        case 1: _t->handleDeselectButton(); break;
        case 2: _t->handleButton(); break;
        case 3: _t->handleBox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->handleBox2((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData NumaChipStats::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject NumaChipStats::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_NumaChipStats,
      qt_meta_data_NumaChipStats, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &NumaChipStats::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *NumaChipStats::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *NumaChipStats::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_NumaChipStats))
        return static_cast<void*>(const_cast< NumaChipStats*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int NumaChipStats::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
static const uint qt_meta_data_PerfGraph[] = {

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

static const char qt_meta_stringdata_PerfGraph[] = {
    "PerfGraph\0\0,on\0showCurve(QwtPlotItem*,bool)\0"
};

void PerfGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PerfGraph *_t = static_cast<PerfGraph *>(_o);
        switch (_id) {
        case 0: _t->showCurve((*reinterpret_cast< QwtPlotItem*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PerfGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PerfGraph::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PerfGraph,
      qt_meta_data_PerfGraph, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PerfGraph::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PerfGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PerfGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PerfGraph))
        return static_cast<void*>(const_cast< PerfGraph*>(this));
    return QWidget::qt_metacast(_clname);
}

int PerfGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_CacheGraph[] = {
    "CacheGraph\0"
};

void CacheGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CacheGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CacheGraph::staticMetaObject = {
    { &PerfGraph::staticMetaObject, qt_meta_stringdata_CacheGraph,
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
    return PerfGraph::qt_metacast(_clname);
}

int CacheGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PerfGraph::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_TransGraph[] = {

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

static const char qt_meta_stringdata_TransGraph[] = {
    "TransGraph\0"
};

void TransGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData TransGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TransGraph::staticMetaObject = {
    { &PerfGraph::staticMetaObject, qt_meta_stringdata_TransGraph,
      qt_meta_data_TransGraph, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TransGraph::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TransGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TransGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TransGraph))
        return static_cast<void*>(const_cast< TransGraph*>(this));
    return PerfGraph::qt_metacast(_clname);
}

int TransGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PerfGraph::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_PerfHistGraph[] = {

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

static const char qt_meta_stringdata_PerfHistGraph[] = {
    "PerfHistGraph\0"
};

void PerfHistGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData PerfHistGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PerfHistGraph::staticMetaObject = {
    { &PerfGraph::staticMetaObject, qt_meta_stringdata_PerfHistGraph,
      qt_meta_data_PerfHistGraph, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PerfHistGraph::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PerfHistGraph::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PerfHistGraph::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PerfHistGraph))
        return static_cast<void*>(const_cast< PerfHistGraph*>(this));
    return PerfGraph::qt_metacast(_clname);
}

int PerfHistGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PerfGraph::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_CacheHistGraph[] = {

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

static const char qt_meta_stringdata_CacheHistGraph[] = {
    "CacheHistGraph\0"
};

void CacheHistGraph::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CacheHistGraph::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CacheHistGraph::staticMetaObject = {
    { &PerfHistGraph::staticMetaObject, qt_meta_stringdata_CacheHistGraph,
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
    return PerfHistGraph::qt_metacast(_clname);
}

int CacheHistGraph::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PerfHistGraph::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_TransactionHist[] = {

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

static const char qt_meta_stringdata_TransactionHist[] = {
    "TransactionHist\0"
};

void TransactionHist::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData TransactionHist::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject TransactionHist::staticMetaObject = {
    { &PerfHistGraph::staticMetaObject, qt_meta_stringdata_TransactionHist,
      qt_meta_data_TransactionHist, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &TransactionHist::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *TransactionHist::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *TransactionHist::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_TransactionHist))
        return static_cast<void*>(const_cast< TransactionHist*>(this));
    return PerfHistGraph::qt_metacast(_clname);
}

int TransactionHist::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PerfHistGraph::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_ProbeHist[] = {

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

static const char qt_meta_stringdata_ProbeHist[] = {
    "ProbeHist\0"
};

void ProbeHist::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData ProbeHist::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject ProbeHist::staticMetaObject = {
    { &PerfHistGraph::staticMetaObject, qt_meta_stringdata_ProbeHist,
      qt_meta_data_ProbeHist, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ProbeHist::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ProbeHist::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ProbeHist::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ProbeHist))
        return static_cast<void*>(const_cast< ProbeHist*>(this));
    return PerfHistGraph::qt_metacast(_clname);
}

int ProbeHist::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = PerfHistGraph::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
