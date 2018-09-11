DEFINES += RSED_ON_QT
DEFINES += RENDERER_IS_SW   # Doesn't assign a renderer, just lets the program which renderer we're using.
#DEFINES += VALIDATION_RUN   # Run RGEO data import/export validation instead of the actual program.

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
CONFIG += console c++11
TARGET = rgeo_linux

OBJECTS_DIR = ../.other/generated_files
MOC_DIR = ../.other/generated_files
UI_DIR = ../.other/generated_files

SOURCES += \
    src/core/file/file.cpp \
    src/core/game_exe/game_exe.cpp \
    src/core/geometry/geometry_sw_matrix.cpp \
    src/core/lookups/lookups.cpp \
    src/core/maasto/maasto.cpp \
    src/core/palat/palat.cpp \
    src/core/palette/palette.cpp \
    src/core/geometry/geometry_sw.cpp \
    src/core/camera/camera.cpp \
    src/dendrons/render/sw/render_sw_texture.cpp \
    src/dendrons/render/sw/render_sw.cpp \
    src/dendrons/render/sw/render_sw_triangle_fill.cpp \
    src/core/text/text.cpp \
    src/core/ray_trace/ray_trace.cpp \
    src/core/frame_timer/frame_timer.cpp \
    src/dendrons/display/qt/display_qt.cpp \
    src/core/manifesto/manifesto.cpp \
    src/core/cmd_line/cmd_line.cpp \
    src/core/props/props.cpp \
    src/core/memory/memory.cpp \
    src/dendrons/display/qt/w_opengl.cpp \
    src/core/ui/ui_layout.cpp \
    src/core/ui/ui_logic.cpp

contains(DEFINES, VALIDATION_RUN) {
    SOURCES += src/core/tests/test_import_export.cpp
} else {
    SOURCES += src/core/main/main.cpp
}


HEADERS += \
    src/core/geometry/geometry_sw.h \
    src/core/common.h \
    src/core/display.h \
    src/core/file.h \
    src/core/fixedpt.h \
    src/core/game_exe.h \
    src/core/geometry.h \
    src/core/maasto.h \
    src/core/palat.h \
    src/core/palette.h \
    src/core/types.h \
    src/core/camera.h \
    src/dendrons/render/sw/render_sw.h \
    src/core/text.h \
    src/core/texture.h \
    src/core/ray_trace.h \
    src/core/frame_timer.h \
    src/dendrons/display/qt/display_qt.h \
    src/core/manifesto.h \
    src/core/cmd_line.h \
    src/core/main.h \
    src/core/tests/reference_container.h \
    src/core/props.h \
    src/core/render.h \
    src/core/memory.h \
    src/dendrons/display/qt/w_opengl.h \
    src/core/ui.h \
    src/core/ui/interactible.h \
    src/core/memory/memory_interface.h

DISTFILES += \
    src/core/render/font.inc \
    src/core/text/font.inc \
    src/core/user/mouse_cursor.inc \
    src/core/ui/mouse_cursor.inc


# C++. For GCC/Clang.
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -ansi
QMAKE_CXXFLAGS += -O2
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -pipe
QMAKE_CXXFLAGS += -pedantic
