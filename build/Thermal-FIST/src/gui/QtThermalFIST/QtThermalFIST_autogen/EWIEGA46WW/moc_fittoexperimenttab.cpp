/****************************************************************************
** Meta object code from reading C++ file 'fittoexperimenttab.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../Thermal-FIST/src/gui/QtThermalFIST/fittoexperimenttab.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'fittoexperimenttab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FitWorker_t {
    QByteArrayData data[3];
    char stringdata0[22];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FitWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FitWorker_t qt_meta_stringdata_FitWorker = {
    {
QT_MOC_LITERAL(0, 0, 9), // "FitWorker"
QT_MOC_LITERAL(1, 10, 10), // "calculated"
QT_MOC_LITERAL(2, 21, 0) // ""

    },
    "FitWorker\0calculated\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FitWorker[] = {

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

void FitWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FitWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->calculated(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FitWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FitWorker::calculated)) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject FitWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_FitWorker.data,
    qt_meta_data_FitWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FitWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FitWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FitWorker.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int FitWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void FitWorker::calculated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_FitToExperimentTab_t {
    QByteArrayData data[31];
    char stringdata0[439];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FitToExperimentTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FitToExperimentTab_t qt_meta_stringdata_FitToExperimentTab = {
    {
QT_MOC_LITERAL(0, 0, 18), // "FitToExperimentTab"
QT_MOC_LITERAL(1, 19, 11), // "writetofile"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 20), // "showValidityCheckLog"
QT_MOC_LITERAL(4, 53, 10), // "changedRow"
QT_MOC_LITERAL(5, 64, 10), // "performFit"
QT_MOC_LITERAL(6, 75, 18), // "ThermalModelConfig"
QT_MOC_LITERAL(7, 94, 6), // "config"
QT_MOC_LITERAL(8, 101, 38), // "thermalfist::ThermalModelFitP..."
QT_MOC_LITERAL(9, 140, 6), // "params"
QT_MOC_LITERAL(10, 147, 9), // "calculate"
QT_MOC_LITERAL(11, 157, 19), // "quantityDoubleClick"
QT_MOC_LITERAL(12, 177, 11), // "QModelIndex"
QT_MOC_LITERAL(13, 189, 11), // "showResults"
QT_MOC_LITERAL(14, 201, 15), // "showChi2Profile"
QT_MOC_LITERAL(15, 217, 8), // "setModel"
QT_MOC_LITERAL(16, 226, 30), // "thermalfist::ThermalModelBase*"
QT_MOC_LITERAL(17, 257, 5), // "model"
QT_MOC_LITERAL(18, 263, 21), // "removeQuantityFromFit"
QT_MOC_LITERAL(19, 285, 11), // "addQuantity"
QT_MOC_LITERAL(20, 297, 12), // "loadFromFile"
QT_MOC_LITERAL(21, 310, 10), // "saveToFile"
QT_MOC_LITERAL(22, 321, 12), // "modelChanged"
QT_MOC_LITERAL(23, 334, 8), // "resetTPS"
QT_MOC_LITERAL(24, 343, 14), // "updateProgress"
QT_MOC_LITERAL(25, 358, 8), // "finalize"
QT_MOC_LITERAL(26, 367, 15), // "updateFontSizes"
QT_MOC_LITERAL(27, 383, 10), // "plotYields"
QT_MOC_LITERAL(28, 394, 14), // "plotDeviations"
QT_MOC_LITERAL(29, 409, 13), // "plotDataModel"
QT_MOC_LITERAL(30, 423, 15) // "plotDataVsModel"

    },
    "FitToExperimentTab\0writetofile\0\0"
    "showValidityCheckLog\0changedRow\0"
    "performFit\0ThermalModelConfig\0config\0"
    "thermalfist::ThermalModelFitParameters\0"
    "params\0calculate\0quantityDoubleClick\0"
    "QModelIndex\0showResults\0showChi2Profile\0"
    "setModel\0thermalfist::ThermalModelBase*\0"
    "model\0removeQuantityFromFit\0addQuantity\0"
    "loadFromFile\0saveToFile\0modelChanged\0"
    "resetTPS\0updateProgress\0finalize\0"
    "updateFontSizes\0plotYields\0plotDeviations\0"
    "plotDataModel\0plotDataVsModel"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FitToExperimentTab[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,  124,    2, 0x08 /* Private */,
       3,    0,  125,    2, 0x08 /* Private */,
       4,    0,  126,    2, 0x0a /* Public */,
       5,    2,  127,    2, 0x0a /* Public */,
      10,    0,  132,    2, 0x0a /* Public */,
      11,    1,  133,    2, 0x0a /* Public */,
      13,    0,  136,    2, 0x0a /* Public */,
      14,    0,  137,    2, 0x0a /* Public */,
      15,    1,  138,    2, 0x0a /* Public */,
      18,    0,  141,    2, 0x0a /* Public */,
      19,    0,  142,    2, 0x0a /* Public */,
      20,    0,  143,    2, 0x0a /* Public */,
      21,    0,  144,    2, 0x0a /* Public */,
      22,    0,  145,    2, 0x0a /* Public */,
      23,    0,  146,    2, 0x0a /* Public */,
      24,    0,  147,    2, 0x0a /* Public */,
      25,    0,  148,    2, 0x0a /* Public */,
      26,    0,  149,    2, 0x0a /* Public */,
      27,    0,  150,    2, 0x0a /* Public */,
      28,    0,  151,    2, 0x0a /* Public */,
      29,    0,  152,    2, 0x0a /* Public */,
      30,    0,  153,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6, 0x80000000 | 8,    7,    9,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 12,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void FitToExperimentTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FitToExperimentTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->writetofile(); break;
        case 1: _t->showValidityCheckLog(); break;
        case 2: _t->changedRow(); break;
        case 3: _t->performFit((*reinterpret_cast< const ThermalModelConfig(*)>(_a[1])),(*reinterpret_cast< const thermalfist::ThermalModelFitParameters(*)>(_a[2]))); break;
        case 4: _t->calculate(); break;
        case 5: _t->quantityDoubleClick((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 6: _t->showResults(); break;
        case 7: _t->showChi2Profile(); break;
        case 8: _t->setModel((*reinterpret_cast< thermalfist::ThermalModelBase*(*)>(_a[1]))); break;
        case 9: _t->removeQuantityFromFit(); break;
        case 10: _t->addQuantity(); break;
        case 11: _t->loadFromFile(); break;
        case 12: _t->saveToFile(); break;
        case 13: _t->modelChanged(); break;
        case 14: _t->resetTPS(); break;
        case 15: _t->updateProgress(); break;
        case 16: _t->finalize(); break;
        case 17: _t->updateFontSizes(); break;
        case 18: _t->plotYields(); break;
        case 19: _t->plotDeviations(); break;
        case 20: _t->plotDataModel(); break;
        case 21: _t->plotDataVsModel(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject FitToExperimentTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_FitToExperimentTab.data,
    qt_meta_data_FitToExperimentTab,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FitToExperimentTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FitToExperimentTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FitToExperimentTab.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int FitToExperimentTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 22;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
