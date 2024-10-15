/****************************************************************************
** Meta object code from reading C++ file 'chi2ProfileDialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../Thermal-FIST/src/gui/QtThermalFIST/chi2ProfileDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'chi2ProfileDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_chi2ProfileWorker_t {
    QByteArrayData data[3];
    char stringdata0[30];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_chi2ProfileWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_chi2ProfileWorker_t qt_meta_stringdata_chi2ProfileWorker = {
    {
QT_MOC_LITERAL(0, 0, 17), // "chi2ProfileWorker"
QT_MOC_LITERAL(1, 18, 10), // "calculated"
QT_MOC_LITERAL(2, 29, 0) // ""

    },
    "chi2ProfileWorker\0calculated\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_chi2ProfileWorker[] = {

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

void chi2ProfileWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<chi2ProfileWorker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->calculated(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (chi2ProfileWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&chi2ProfileWorker::calculated)) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject chi2ProfileWorker::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_chi2ProfileWorker.data,
    qt_meta_data_chi2ProfileWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *chi2ProfileWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *chi2ProfileWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_chi2ProfileWorker.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int chi2ProfileWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void chi2ProfileWorker::calculated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_chi2ProfileDialog_t {
    QByteArrayData data[10];
    char stringdata0[109];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_chi2ProfileDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_chi2ProfileDialog_t qt_meta_stringdata_chi2ProfileDialog = {
    {
QT_MOC_LITERAL(0, 0, 17), // "chi2ProfileDialog"
QT_MOC_LITERAL(1, 18, 9), // "calculate"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 6), // "replot"
QT_MOC_LITERAL(4, 36, 8), // "finalize"
QT_MOC_LITERAL(5, 45, 14), // "updateProgress"
QT_MOC_LITERAL(6, 60, 16), // "parameterChanged"
QT_MOC_LITERAL(7, 77, 5), // "index"
QT_MOC_LITERAL(8, 83, 13), // "limitsChanged"
QT_MOC_LITERAL(9, 97, 11) // "writetoFile"

    },
    "chi2ProfileDialog\0calculate\0\0replot\0"
    "finalize\0updateProgress\0parameterChanged\0"
    "index\0limitsChanged\0writetoFile"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_chi2ProfileDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x0a /* Public */,
       3,    0,   50,    2, 0x0a /* Public */,
       4,    0,   51,    2, 0x0a /* Public */,
       5,    0,   52,    2, 0x0a /* Public */,
       6,    1,   53,    2, 0x0a /* Public */,
       8,    0,   56,    2, 0x0a /* Public */,
       9,    0,   57,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    7,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void chi2ProfileDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<chi2ProfileDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->calculate(); break;
        case 1: _t->replot(); break;
        case 2: _t->finalize(); break;
        case 3: _t->updateProgress(); break;
        case 4: _t->parameterChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->limitsChanged(); break;
        case 6: _t->writetoFile(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject chi2ProfileDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_chi2ProfileDialog.data,
    qt_meta_data_chi2ProfileDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *chi2ProfileDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *chi2ProfileDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_chi2ProfileDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int chi2ProfileDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
