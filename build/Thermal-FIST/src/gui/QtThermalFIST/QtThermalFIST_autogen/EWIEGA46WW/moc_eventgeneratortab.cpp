/****************************************************************************
** Meta object code from reading C++ file 'eventgeneratortab.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../Thermal-FIST/src/gui/QtThermalFIST/eventgeneratortab.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'eventgeneratortab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_EventGeneratorWorker_t {
    QByteArrayData data[3];
    char stringdata0[33];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_EventGeneratorWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_EventGeneratorWorker_t qt_meta_stringdata_EventGeneratorWorker = {
    {
QT_MOC_LITERAL(0, 0, 20), // "EventGeneratorWorker"
QT_MOC_LITERAL(1, 21, 10), // "calculated"
QT_MOC_LITERAL(2, 32, 0) // ""

    },
    "EventGeneratorWorker\0calculated\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_EventGeneratorWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,

       0        // eod
};

void EventGeneratorWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EventGeneratorWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->calculated(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (EventGeneratorWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EventGeneratorWorker::calculated)) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject EventGeneratorWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_EventGeneratorWorker.data,
    qt_meta_data_EventGeneratorWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *EventGeneratorWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EventGeneratorWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_EventGeneratorWorker.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int EventGeneratorWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void EventGeneratorWorker::calculated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_BinningDialog_t {
    QByteArrayData data[4];
    char stringdata0[26];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_BinningDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_BinningDialog_t qt_meta_stringdata_BinningDialog = {
    {
QT_MOC_LITERAL(0, 0, 13), // "BinningDialog"
QT_MOC_LITERAL(1, 14, 2), // "OK"
QT_MOC_LITERAL(2, 17, 0), // ""
QT_MOC_LITERAL(3, 18, 7) // "Discard"

    },
    "BinningDialog\0OK\0\0Discard"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BinningDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x0a /* Public */,
       3,    0,   25,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void BinningDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BinningDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->OK(); break;
        case 1: _t->Discard(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject BinningDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_BinningDialog.data,
    qt_meta_data_BinningDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *BinningDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BinningDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_BinningDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int BinningDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
struct qt_meta_stringdata_ParticlesAnalyzeDialog_t {
    QByteArrayData data[4];
    char stringdata0[35];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ParticlesAnalyzeDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ParticlesAnalyzeDialog_t qt_meta_stringdata_ParticlesAnalyzeDialog = {
    {
QT_MOC_LITERAL(0, 0, 22), // "ParticlesAnalyzeDialog"
QT_MOC_LITERAL(1, 23, 2), // "OK"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 7) // "Discard"

    },
    "ParticlesAnalyzeDialog\0OK\0\0Discard"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ParticlesAnalyzeDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x0a /* Public */,
       3,    0,   25,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ParticlesAnalyzeDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ParticlesAnalyzeDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->OK(); break;
        case 1: _t->Discard(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject ParticlesAnalyzeDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ParticlesAnalyzeDialog.data,
    qt_meta_data_ParticlesAnalyzeDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ParticlesAnalyzeDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ParticlesAnalyzeDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ParticlesAnalyzeDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ParticlesAnalyzeDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
struct qt_meta_stringdata_EventGeneratorTab_t {
    QByteArrayData data[48];
    char stringdata0[534];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_EventGeneratorTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_EventGeneratorTab_t qt_meta_stringdata_EventGeneratorTab = {
    {
QT_MOC_LITERAL(0, 0, 17), // "EventGeneratorTab"
QT_MOC_LITERAL(1, 18, 10), // "changedRow"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 9), // "calculate"
QT_MOC_LITERAL(4, 40, 6), // "replot"
QT_MOC_LITERAL(5, 47, 15), // "QVector<double>"
QT_MOC_LITERAL(6, 63, 2), // "x1"
QT_MOC_LITERAL(7, 66, 2), // "y1"
QT_MOC_LITERAL(8, 69, 5), // "y1err"
QT_MOC_LITERAL(9, 75, 2), // "x2"
QT_MOC_LITERAL(10, 78, 2), // "y2"
QT_MOC_LITERAL(11, 81, 5), // "index"
QT_MOC_LITERAL(12, 87, 10), // "rightlimit"
QT_MOC_LITERAL(13, 98, 8), // "replot2D"
QT_MOC_LITERAL(14, 107, 2), // "xv"
QT_MOC_LITERAL(15, 110, 2), // "yv"
QT_MOC_LITERAL(16, 113, 2), // "zv"
QT_MOC_LITERAL(17, 116, 19), // "quantityDoubleClick"
QT_MOC_LITERAL(18, 136, 11), // "QModelIndex"
QT_MOC_LITERAL(19, 148, 8), // "setModel"
QT_MOC_LITERAL(20, 157, 30), // "thermalfist::ThermalModelBase*"
QT_MOC_LITERAL(21, 188, 5), // "model"
QT_MOC_LITERAL(22, 194, 14), // "updateProgress"
QT_MOC_LITERAL(23, 209, 8), // "finalize"
QT_MOC_LITERAL(24, 218, 10), // "changePlot"
QT_MOC_LITERAL(25, 229, 12), // "modelChanged"
QT_MOC_LITERAL(26, 242, 8), // "resetTPS"
QT_MOC_LITERAL(27, 251, 14), // "loadAcceptance"
QT_MOC_LITERAL(28, 266, 16), // "chooseOutputFile"
QT_MOC_LITERAL(29, 283, 15), // "changeVolumeRSC"
QT_MOC_LITERAL(30, 299, 10), // "changeTkin"
QT_MOC_LITERAL(31, 310, 14), // "generateEvents"
QT_MOC_LITERAL(32, 325, 18), // "ThermalModelConfig"
QT_MOC_LITERAL(33, 344, 6), // "config"
QT_MOC_LITERAL(34, 351, 24), // "contextMenuRequestPlot1D"
QT_MOC_LITERAL(35, 376, 3), // "pos"
QT_MOC_LITERAL(36, 380, 11), // "saveAsPdf1D"
QT_MOC_LITERAL(37, 392, 11), // "saveAsPng1D"
QT_MOC_LITERAL(38, 404, 13), // "saveAsAscii1D"
QT_MOC_LITERAL(39, 418, 8), // "saveAs1D"
QT_MOC_LITERAL(40, 427, 4), // "type"
QT_MOC_LITERAL(41, 432, 24), // "contextMenuRequestPlot2D"
QT_MOC_LITERAL(42, 457, 11), // "saveAsPdf2D"
QT_MOC_LITERAL(43, 469, 11), // "saveAsPng2D"
QT_MOC_LITERAL(44, 481, 13), // "saveAsAscii2D"
QT_MOC_LITERAL(45, 495, 8), // "saveAs2D"
QT_MOC_LITERAL(46, 504, 15), // "changeParticles"
QT_MOC_LITERAL(47, 520, 13) // "changeBinning"

    },
    "EventGeneratorTab\0changedRow\0\0calculate\0"
    "replot\0QVector<double>\0x1\0y1\0y1err\0"
    "x2\0y2\0index\0rightlimit\0replot2D\0xv\0"
    "yv\0zv\0quantityDoubleClick\0QModelIndex\0"
    "setModel\0thermalfist::ThermalModelBase*\0"
    "model\0updateProgress\0finalize\0changePlot\0"
    "modelChanged\0resetTPS\0loadAcceptance\0"
    "chooseOutputFile\0changeVolumeRSC\0"
    "changeTkin\0generateEvents\0ThermalModelConfig\0"
    "config\0contextMenuRequestPlot1D\0pos\0"
    "saveAsPdf1D\0saveAsPng1D\0saveAsAscii1D\0"
    "saveAs1D\0type\0contextMenuRequestPlot2D\0"
    "saveAsPdf2D\0saveAsPng2D\0saveAsAscii2D\0"
    "saveAs2D\0changeParticles\0changeBinning"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_EventGeneratorTab[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      31,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  169,    2, 0x0a /* Public */,
       3,    0,  170,    2, 0x0a /* Public */,
       4,    0,  171,    2, 0x0a /* Public */,
       4,    7,  172,    2, 0x0a /* Public */,
       4,    6,  187,    2, 0x2a /* Public | MethodCloned */,
      13,    5,  200,    2, 0x0a /* Public */,
      13,    4,  211,    2, 0x2a /* Public | MethodCloned */,
      17,    1,  220,    2, 0x0a /* Public */,
      19,    1,  223,    2, 0x0a /* Public */,
      22,    0,  226,    2, 0x0a /* Public */,
      23,    0,  227,    2, 0x0a /* Public */,
      24,    0,  228,    2, 0x0a /* Public */,
      25,    0,  229,    2, 0x0a /* Public */,
      26,    0,  230,    2, 0x0a /* Public */,
      27,    0,  231,    2, 0x0a /* Public */,
      28,    0,  232,    2, 0x0a /* Public */,
      29,    1,  233,    2, 0x0a /* Public */,
      30,    1,  236,    2, 0x0a /* Public */,
      31,    1,  239,    2, 0x0a /* Public */,
      34,    1,  242,    2, 0x0a /* Public */,
      36,    0,  245,    2, 0x0a /* Public */,
      37,    0,  246,    2, 0x0a /* Public */,
      38,    0,  247,    2, 0x0a /* Public */,
      39,    1,  248,    2, 0x0a /* Public */,
      41,    1,  251,    2, 0x0a /* Public */,
      42,    0,  254,    2, 0x0a /* Public */,
      43,    0,  255,    2, 0x0a /* Public */,
      44,    0,  256,    2, 0x0a /* Public */,
      45,    1,  257,    2, 0x0a /* Public */,
      46,    0,  260,    2, 0x0a /* Public */,
      47,    0,  261,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5, QMetaType::Int, QMetaType::Double,    6,    7,    8,    9,   10,   11,   12,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5, QMetaType::Int,    6,    7,    8,    9,   10,   11,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5, QMetaType::Int, QMetaType::Double,   14,   15,   16,   11,   12,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 5, 0x80000000 | 5, QMetaType::Int,   14,   15,   16,   11,
    QMetaType::Void, 0x80000000 | 18,    2,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, 0x80000000 | 32,   33,
    QMetaType::Void, QMetaType::QPoint,   35,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   40,
    QMetaType::Void, QMetaType::QPoint,   35,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   40,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void EventGeneratorTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EventGeneratorTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->changedRow(); break;
        case 1: _t->calculate(); break;
        case 2: _t->replot(); break;
        case 3: _t->replot((*reinterpret_cast< const QVector<double>(*)>(_a[1])),(*reinterpret_cast< const QVector<double>(*)>(_a[2])),(*reinterpret_cast< const QVector<double>(*)>(_a[3])),(*reinterpret_cast< const QVector<double>(*)>(_a[4])),(*reinterpret_cast< const QVector<double>(*)>(_a[5])),(*reinterpret_cast< int(*)>(_a[6])),(*reinterpret_cast< double(*)>(_a[7]))); break;
        case 4: _t->replot((*reinterpret_cast< const QVector<double>(*)>(_a[1])),(*reinterpret_cast< const QVector<double>(*)>(_a[2])),(*reinterpret_cast< const QVector<double>(*)>(_a[3])),(*reinterpret_cast< const QVector<double>(*)>(_a[4])),(*reinterpret_cast< const QVector<double>(*)>(_a[5])),(*reinterpret_cast< int(*)>(_a[6]))); break;
        case 5: _t->replot2D((*reinterpret_cast< const QVector<double>(*)>(_a[1])),(*reinterpret_cast< const QVector<double>(*)>(_a[2])),(*reinterpret_cast< const QVector<double>(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< double(*)>(_a[5]))); break;
        case 6: _t->replot2D((*reinterpret_cast< const QVector<double>(*)>(_a[1])),(*reinterpret_cast< const QVector<double>(*)>(_a[2])),(*reinterpret_cast< const QVector<double>(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 7: _t->quantityDoubleClick((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 8: _t->setModel((*reinterpret_cast< thermalfist::ThermalModelBase*(*)>(_a[1]))); break;
        case 9: _t->updateProgress(); break;
        case 10: _t->finalize(); break;
        case 11: _t->changePlot(); break;
        case 12: _t->modelChanged(); break;
        case 13: _t->resetTPS(); break;
        case 14: _t->loadAcceptance(); break;
        case 15: _t->chooseOutputFile(); break;
        case 16: _t->changeVolumeRSC((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 17: _t->changeTkin((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 18: _t->generateEvents((*reinterpret_cast< const ThermalModelConfig(*)>(_a[1]))); break;
        case 19: _t->contextMenuRequestPlot1D((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 20: _t->saveAsPdf1D(); break;
        case 21: _t->saveAsPng1D(); break;
        case 22: _t->saveAsAscii1D(); break;
        case 23: _t->saveAs1D((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 24: _t->contextMenuRequestPlot2D((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 25: _t->saveAsPdf2D(); break;
        case 26: _t->saveAsPng2D(); break;
        case 27: _t->saveAsAscii2D(); break;
        case 28: _t->saveAs2D((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 29: _t->changeParticles(); break;
        case 30: _t->changeBinning(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 4:
            case 3:
            case 2:
            case 1:
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<double> >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 4:
            case 3:
            case 2:
            case 1:
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<double> >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 2:
            case 1:
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<double> >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 2:
            case 1:
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<double> >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject EventGeneratorTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_EventGeneratorTab.data,
    qt_meta_data_EventGeneratorTab,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *EventGeneratorTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EventGeneratorTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_EventGeneratorTab.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int EventGeneratorTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
