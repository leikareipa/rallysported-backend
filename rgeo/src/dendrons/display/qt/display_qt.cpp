/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED, Qt-based window.
 *
 */

#include <QOpenGLWidget>
#include <QElapsedTimer>
#include <QApplication>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QWidget>
#include <QDebug>
#include "../../../dendrons/display/qt/display_qt.h"
#include "../../../dendrons/display/qt/w_opengl.h"
#include "../../../core/display.h"
#include "../../../core/render.h"
#include "../../../core/main.h"
#include "../../../core/ui.h"

/*
 * TODOS:
 *
 * - (none at the moment.)
 *
 */

// The window we'll display the program in.
static Window *WINDOW = nullptr;
static OGLWidget *OGL_SURFACE = nullptr;

// How many times per second the display is being updated.
static uint FPS = 0;

// For the Qt GUI to work, we need to have a QApplication object around. We just
// pass some dummy args to it.
namespace app_n
{
    static char NAME[] = "\"RallySportED\" by Tarpeeksi Hyvae Soft";
    static int ARGC = 1;
    static char *ARGV = NAME;
    static QApplication *const APP = new QApplication(ARGC, &ARGV);
}

void kd_set_display_palette(const color_rgb_s *const p)
{
    /// Not used.

    (void)p;

    return;
}

void kd_acquire_display(void)
{
    DEBUG(("Acquiring the display."));

    // Set the OpenGL surface format.
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(1, 2);
    format.setSwapInterval(1); // Vsync.
    format.setSamples(0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    const resolution_s r = kuil_ideal_display_resolution(false);
    WINDOW = new Window(r.w, r.h);

    return;
}

void kd_release_display(void)
{
    DEBUG(("Freeing the display."));

    delete WINDOW;
    delete app_n::APP;

    return;
}

uint kd_current_fps(void)
{
    return FPS;
}

void kd_show_headless_info_message(const char *const msg)
{
    QMessageBox mb;
    mb.setWindowTitle("A message from RallySportED");
    mb.setText(msg);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setIcon(QMessageBox::Information);
    mb.setDefaultButton(QMessageBox::Ok);

    mb.exec();

    return;
}

void kd_show_headless_warning_message(const char *const msg)
{
    QMessageBox mb;
    mb.setWindowTitle("A warning from RallySportED");
    mb.setText(msg);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setIcon(QMessageBox::Warning);
    mb.setDefaultButton(QMessageBox::Ok);

    mb.exec();

    return;
}

void kd_show_headless_assert_error_message(const char *const msg)
{
    QMessageBox mb;
    mb.setWindowTitle("RallySportED Assertion Fail");
    mb.setText("RallySportED has come across an unexpected condition in its code. As a precaution, "
               "it will shut down.\n\nThe following cause of error was provided:\n\""
               + QString(msg) + "\"\n\nIf you ran the program from a console window, further "
               "diagnostics should appear there after termination.");
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setIcon(QMessageBox::Critical);
    mb.setDefaultButton(QMessageBox::Ok);

    mb.exec();

    return;
}

void kd_update_display(void)
{
    static u32 fpsCnt = 0;

    static QElapsedTimer fpsTimer;
    if (!fpsTimer.isValid())
    {
        fpsTimer.start();
    }

    fpsCnt++;
    if (fpsTimer.elapsed() >= 1000)
    {
        FPS = fpsCnt;
        fpsCnt = 0;
        fpsTimer.restart();
    }

    WINDOW->update();   // Will call paintGL(), which draws the renderer's current frame buffer to screen.

    // Spin the event loop manually, relying on OpenGL's refresh block to keep us
    // in sync with the monitor's refresh rate.
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents();

    return;
}

resolution_s kd_display_resolution(void)
{
    const resolution_s r = {(uint)WINDOW->size().width(), (uint)WINDOW->size().height(), 32};

    return r;
}

vector2<real> kd_cursor_position(void)
{
    vector2<real> c;
    const QPointF cursorPos = WINDOW->CursorPosition();

    c.x = cursorPos.x();
    c.y = cursorPos.y();

    return c;
}

// Target the given FPS by waiting around until the equivalent number of milliseconds
// has passed. Call this function once per frame to achieve the desired result.
// If the given fps is 0, the function will not wait at all.
//
void kd_target_fps(const uint fps)
{
    /// Does nothing; we rely on OpenGL to keep the fps locked to refresh.

    (void)fps;

    return;
}

Window::Window(const uint width, const uint height)
{
    // Initialize the window.
    this->setAttribute(Qt::WA_OpaquePaintEvent, true);  // We'll repaint the entire window every time.
    this->setCursor(Qt::BlankCursor); // We'll draw our own cursor.
    this->setMouseTracking(true);
    this->installEventFilter(this);
    this->resize(width, height);
    this->setFixedSize(size());

    // Initialize OpenGL.
    OGL_SURFACE = new OGLWidget(this);
    OGL_SURFACE->resize(this->size());

    this->show();

    return;
}

Window::~Window()
{
    return;
}

QPointF Window::CursorPosition()
{
    QPointF cursorPos = mapFromGlobal(QCursor::pos());

    return cursorPos;
}

bool Window::eventFilter(QObject *, QEvent *event)
{
    // Let the engine know where the mouse cursor is located.
    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *e = (QMouseEvent*)event;

        switch (e->button())
        {
            case Qt::LeftButton:    { kui_set_mouse_button_left_pressed(true);           break; }
            case Qt::RightButton:   { kui_set_mouse_button_right_pressed(true);          break; }
            case Qt::MidButton:     { kui_set_mouse_button_middle_pressed(true);         break; }
            default: { DEBUG(("Detected an unknown mouse button pressed. Ignoring it."));   break; }
        }

        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent *e = (QMouseEvent*)event;

        switch (e->button())
        {
            case Qt::LeftButton:    { kui_set_mouse_button_left_pressed(false);          break; }
            case Qt::RightButton:   { kui_set_mouse_button_right_pressed(false);         break; }
            case Qt::MidButton:     { kui_set_mouse_button_middle_pressed(false);        break; }
            default: { DEBUG(("Detected an unknown mouse button release. Ignoring it."));   break; }
        }

        return true;
    }
    // Keyboard.
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *e = (QKeyEvent*)event;

        if (e->isAutoRepeat()) // We only want one event per keypress.
        {
            return true;
        }

        switch (e->key())
        {
            case Qt::Key_Tab:       { kui_set_key_pressed(INPUT_KEY_PANE);      break; }
            case Qt::Key_Escape:    { kui_set_key_pressed(INPUT_KEY_ESC);       break; }
            case Qt::Key_F1:        { kui_set_key_pressed(INPUT_KEY_PAINT);     break; }
            case Qt::Key_F2:        { kui_set_key_pressed(INPUT_KEY_PALA);      break; }
            case Qt::Key_F5:        { kui_set_key_pressed(INPUT_KEY_REFRESH);   break; }
            case Qt::Key_Shift:     { kui_set_key_pressed(INPUT_KEY_WIREFRAME); break; }
            case Qt::Key_Space:     { kui_set_key_pressed(INPUT_KEY_SPACE);     break; }
            case Qt::Key_F:         { kui_set_key_pressed(INPUT_KEY_FPS);       break; }
            case Qt::Key_K:         { kui_set_key_pressed(INPUT_KEY_SAVE);      break; }
            case Qt::Key_C:         { kui_set_key_pressed(INPUT_KEY_CAMERA);    break; }
            case Qt::Key_W:         { kui_set_key_pressed(INPUT_KEY_WATER);     break; }
            case Qt::Key_1:         { kui_set_key_pressed(INPUT_KEY_1);         break; }
            case Qt::Key_2:         { kui_set_key_pressed(INPUT_KEY_2);         break; }
            case Qt::Key_3:         { kui_set_key_pressed(INPUT_KEY_3);         break; }
            case Qt::Key_4:         { kui_set_key_pressed(INPUT_KEY_4);         break; }
            case Qt::Key_5:         { kui_set_key_pressed(INPUT_KEY_5);         break; }
            case Qt::Key_0:         { kui_set_key_pressed(INPUT_KEY_EXIT);      break; }

            default: { DEBUG(("Detected an unknown keyboard key press (%d). Ignoring it.", e->key())); break; }
        }

        return true;
    }

    return false;
}

void Window::closeEvent(QCloseEvent *)
{
    DEBUG(("Have been asked to close the window."));

    kmain_request_program_exit(EXIT_SUCCESS);

    return;
}
