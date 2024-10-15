/****************************************************************************
** Meta object code from reading C++ file 'equationofstatetab.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../Thermal-FIST/src/gui/QtThermalFIST/equationofstatetab.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'equationofstatetab.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_EoSWorker_t {
    QByteArrayData data[3];
    char stringdata0[22];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_EoSWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_EoSWorker_t qt_meta_stringdata_EoSWorker = {
    {
QT_MOC_LITERAL(0, 0, 9), // "EoSWorker"
QT_MOC_LITERAL(1, 10, 10), // "calculated"
QT_MOC_LITERAL(2, 21, 0) // ""

    },
    "EoSWorker\0calculated\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_EoSWorker[] = {

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

void EoSWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EoSWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->calculated(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (EoSWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&EoSWorker::calculated)) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject EoSWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_EoSWorker.data,
    qt_meta_data_EoSWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *EoSWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EoSWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_EoSWorker.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int EoSWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void EoSWorker::calculated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_EquationOfStateTab_t {
    QByteArrayData data[16];
    char stringdata0[169];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_EquationOfStateTab_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_EquationOfStateTab_t qt_meta_stringdata_EquationOfStateTab = {
    {
QT_MOC_LITERAL(0, 0, 18), // "EquationOfStateTab"
QT_MOC_LITERAL(1, 19, 9), // "calculate"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 6), // "replot"
QT_MOC_LITERAL(4, 37, 8), // "finalize"
QT_MOC_LITERAL(5, 46, 12), // "modelChanged"
QT_MOC_LITERAL(6, 59, 8), // "resetTPS"
QT_MOC_LITERAL(7, 68, 15), // "plotLatticeData"
QT_MOC_LITERAL(8, 84, 17), // "fillParticleLists"
QT_MOC_LITERAL(9, 102, 18), // "contextMenuRequest"
QT_MOC_LITERAL(10, 121, 3), // "pos"
QT_MOC_LITERAL(11, 125, 9), // "saveAsPdf"
QT_MOC_LITERAL(12, 135, 9), // "saveAsPng"
QT_MOC_LITERAL(13, 145, 11), // "saveAsAscii"
QT_MOC_LITERAL(14, 157, 6), // "saveAs"
QT_MOC_LITERAL(15, 164, 4) // "type"

    },
    "EquationOfStateTab\0calculate\0\0replot\0"
    "finalize\0modelChanged\0resetTPS\0"
    "plotLatticeData\0fillParticleLists\0"
    "contextMenuRequest\0pos\0saveAsPdf\0"
    "saveAsPng\0saveAsAscii\0saveAs\0type"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_EquationOfStateTab[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x0a /* Public */,
       3,    0,   75,    2, 0x0a /* Public */,
       4,    0,   76,    2, 0x0a /* Public */,
       5,    0,   77,    2, 0x0a /* Public */,
       6,    0,   78,    2, 0x0a /* Public */,
       7,    0,   79,    2, 0x0a /* Public */,
       8,    0,   80,    2, 0x0a /* Public */,
       9,    1,   81,    2, 0x0a /* Public */,
      11,    0,   84,    2, 0x0a /* Public */,
      12,    0,   85,    2, 0x0a /* Public */,
      13,    0,   86,    2, 0x0a /* Public */,
      14,    1,   87,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,

       0        // eod
};

void EquationOfStateTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<EquationOfStateTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->calculate(); break;
        case 1: _t->replot(); break;
        case 2: _t->finalize(); break;
        case 3: _t->modelChanged(); break;
        case 4: _t->resetTPS(); break;
        case 5: _t->plotLatticeData(); break;
        case 6: _t->fillParticleLists(); break;
        case 7: _t->contextMenuRequest((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 8: _t->saveAsPdf(); break;
        case 9: _t->saveAsPng(); break;
        case 10: _t->saveAsAscii(); break;
        case 11: _t->saveAs((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject EquationOfStateTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_EquationOfStateTab.data,
    qt_meta_data_EquationOfStateTab,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *EquationOfStateTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *EquationOfStateTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_EquationOfStateTab.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int EquationOfStateTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
