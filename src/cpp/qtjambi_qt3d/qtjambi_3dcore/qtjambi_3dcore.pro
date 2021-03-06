QTJAMBILIB = QtJambi3DCore
TARGET = $$QTJAMBILIB

VERSION = $$QT_VERSION

include(../../qtjambi/qtjambi_include.pri)
include(../$$QTJAMBI_CPP/$$QTJAMBILIB/generated.pri)
DESTDIR = ../../lib
DLLDESTDIR = ../../bin

QT += core gui 3dcore 3dcore-private

macx:{
    INCLUDEPATH += $$(QTDIR)/lib/Qt3DCore.framework/Headers/$$QT_VERSION/Qt3DCore/private
}
INCLUDEPATH += $$(QTDIR)/include/Qt3DCore/$$QT_VERSION/Qt3DCore/private


HEADERS += \
    qtjambi_3dcore.h
