/****************************************************************************
** Meta object code from reading C++ file 'particledialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../../../Thermal-FIST/src/gui/QtThermalFIST/particledialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'particledialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ParticleDialog_t {
    QByteArrayData data[11];
    char stringdata0[140];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ParticleDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ParticleDialog_t qt_meta_stringdata_ParticleDialog = {
    {
QT_MOC_LITERAL(0, 0, 14), // "ParticleDialog"
QT_MOC_LITERAL(1, 15, 17), // "checkFixTableSize"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 8), // "addDecay"
QT_MOC_LITERAL(4, 43, 11), // "removeDecay"
QT_MOC_LITERAL(5, 55, 9), // "addColumn"
QT_MOC_LITERAL(6, 65, 12), // "removeColumn"
QT_MOC_LITERAL(7, 78, 20), // "showSpectralFunction"
QT_MOC_LITERAL(8, 99, 18), // "contextMenuRequest"
QT_MOC_LITERAL(9, 118, 3), // "pos"
QT_MOC_LITERAL(10, 122, 17) // "copyFeeddownTable"

    },
    "ParticleDialog\0checkFixTableSize\0\0"
    "addDecay\0removeDecay\0addColumn\0"
    "removeColumn\0showSpectralFunction\0"
    "contextMenuRequest\0pos\0copyFeeddownTable"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ParticleDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x0a /* Public */,
       3,    0,   55,    2, 0x0a /* Public */,
       4,    0,   56,    2, 0x0a /* Public */,
       5,    0,   57,    2, 0x0a /* Public */,
       6,    0,   58,    2, 0x0a /* Public */,
       7,    0,   59,    2, 0x0a /* Public */,
       8,    1,   60,    2, 0x0a /* Public */,
      10,    0,   63,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QPoint,    9,
    QMetaType::Void,

       0        // eod
};

void ParticleDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ParticleDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->checkFixTableSize(); break;
        case 1: _t->addDecay(); break;
        case 2: _t->removeDecay(); break;
        case 3: _t->addColumn(); break;
        case 4: _t->removeColumn(); break;
        case 5: _t->showSpectralFunction(); break;
        case 6: _t->contextMenuRequest((*reinterpret_cast< QPoint(*)>(_a[1]))); break;
        case 7: _t->copyFeeddownTable(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ParticleDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_ParticleDialog.data,
    qt_meta_data_ParticleDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ParticleDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ParticleDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ParticleDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int ParticleDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
