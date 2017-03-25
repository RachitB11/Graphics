TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    mat4.cpp \
    vec4.cpp \
    tiny_obj_loader.cc \
    raster_tools.cpp

HEADERS += \
    mat4.h \
    vec4.h \
    tiny_obj_loader.h \
    raster_tools.h

DISTFILES += \
    cube.obj \
    dodecahedron.obj \
    square.obj \
    square_big.obj \
    wahoo.obj \
    camera.txt \
    extra.txt
