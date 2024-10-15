/****************************************************************************
** Meta object code from reading C++ file 'modeltab.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../Thermal-FIST/src/gui/QtThermalFIST/modeltab.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'modeltab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ModelTab_t {
    QByteArrayData data[22];
    char stringdata0[322];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ModelTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ModelTab_t qt_meta_stringdata_ModelTab = {
    {
QT_MOC_LITERAL(0, 0, 8), // "ModelTab"
QT_MOC_LITERAL(1, 9, 10), // "changedRow"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 18), // "performCalculation"
QT_MOC_LITERAL(4, 40, 18), // "ThermalModelConfig"
QT_MOC_LITERAL(5, 59, 6), // "config"
QT_MOC_LITERAL(6, 66, 9), // "calculate"
QT_MOC_LITERAL(7, 76, 15), // "calculateFitted"
QT_MOC_LITERAL(8, 92, 11), // "writetofile"
QT_MOC_LITERAL(9, 104, 9), // "benchmark"
QT_MOC_LITERAL(10, 114, 23), // "particleInfoDoubleClick"
QT_MOC_LITERAL(11, 138, 11), // "QModelIndex"
QT_MOC_LITERAL(12, 150, 15), // "switchStability"
QT_MOC_LITERAL(13, 166, 11), // "showResults"
QT_MOC_LITERAL(14, 178, 16), // "showCorrelations"
QT_MOC_LITERAL(15, 195, 8), // "setModel"
QT_MOC_LITERAL(16, 204, 30), // "thermalfist::ThermalModelBase*"
QT_MOC_LITERAL(17, 235, 5), // "model"
QT_MOC_LITERAL(18, 241, 12), // "modelChanged"
QT_MOC_LITERAL(19, 254, 15), // "changeVolumeRSC"
QT_MOC_LITERAL(20, 270, 20), // "showValidityCheckLog"
QT_MOC_LITERAL(21, 291, 30) // "computeHigherOrderFluctuations"

    },
    "ModelTab\0changedRow\0\0performCalculation\0"
    "ThermalModelConfig\0config\0calculate\0"
    "calculateFitted\0writetofile\0benchmark\0"
    "particleInfoDoubleClick\0QModelIndex\0"
    "switchStability\0showResults\0"
    "showCorrelations\0setModel\0"
    "thermalfist::ThermalModelBase*\0model\0"
    "modelChanged\0changeVolumeRSC\0"
    "showValidityCheckLog\0"
    "computeHigherOrderFluctuations"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ModelTab[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   89,    2, 0x08 /* Private */,
       3,    1,   90,    2, 0x08 /* Private */,
       6,    0,   93,    2, 0x08 /* Private */,
       7,    0,   94,    2, 0x08 /* Private */,
       8,    0,   95,    2, 0x08 /* Private */,
       9,    0,   96,    2, 0x08 /* Private */,
      10,    1,   97,    2, 0x08 /* Private */,
      12,    1,  100,    2, 0x08 /* Private */,
      13,    0,  103,    2, 0x08 /* Private */,
      14,    0,  104,    2, 0x08 /* Private */,
      15,    1,  105,    2, 0x08 /* Private */,
      18,    0,  108,    2, 0x08 /* Private */,
      19,    1,  109,    2, 0x08 /* Private */,
      20,    0,  112,    2, 0x08 /* Private */,
      21,    0,  113,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ModelTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ModelTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->changedRow(); break;
        case 1: _t->performCalculation((*reinterpret_cast< const ThermalModelConfig(*)>(_a[1]))); break;
        case 2: _t->calculate(); break;
        case 3: _t->calculateFitted(); break;
        case 4: _t->writetofile(); break;
        case 5: _t->benchmark(); break;
        case 6: _t->particleInfoDoubleClick((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 7: _t->switchStability((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->showResults(); break;
        case 9: _t->showCorrelations(); break;
        case 10: _t->setModel((*reinterpret_cast< thermalfist::ThermalModelBase*(*)>(_a[1]))); break;
        case 11: _t->modelChanged(); break;
        case 12: _t->changeVolumeRSC((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 13: _t->showValidityCheckLog(); break;
        case 14: _t->computeHigherOrderFluctuations(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ModelTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ModelTab.data,
    qt_meta_data_ModelTab,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ModelTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ModelTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ModelTab.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ModelTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 15;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
