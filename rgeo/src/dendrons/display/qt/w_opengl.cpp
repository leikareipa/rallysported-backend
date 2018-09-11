/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 * Uses Qt's OpenGL implementation to draw the contents of RGEO's render framebuffer
 * to screen. I.e. just draws a full-window textured quad.
 *
 */

#include <QOpenGLFunctions>
#include <QCoreApplication>
#include <QOpenGLWidget>
#include <QMatrix4x4>
#include "../../../dendrons/display/qt/w_opengl.h"
#include "../../../core/display.h"
#include "../../../core/render.h"

GLuint FRAMEBUFFER_TEXTURE;

OGLWidget::OGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    return;
}

void OGLWidget::initializeGL()
{
    DEBUG(("Initializing OpenGL..."));

    this->initializeOpenGLFunctions();

    DEBUG(("OpenGL is reported to be version %s.", glGetString(GL_VERSION)));

    this->setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    this->glDisable(GL_DEPTH_TEST);
    this->glEnable(GL_TEXTURE_2D);

    // Initialize the texture into which we'll stream the renderer's framebuffer.
    this->glGenTextures(1, &FRAMEBUFFER_TEXTURE);
    this->glBindTexture(GL_TEXTURE_2D, FRAMEBUFFER_TEXTURE);
    this->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    this->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    k_assert(!glGetError(), "OpenGL initialization failed.");

    return;
}

void OGLWidget::resizeGL(int w, int h)
{
    QMatrix4x4 m;
    m.setToIdentity();
    m.ortho(0, w, h, 0, -1, 1);

    glLoadMatrixf(m.constData());

    return;
}

void OGLWidget::paintGL()
{
    const frame_buffer_s *const fb = kr_framebuffer_ptr();
    if (fb->canvas == nullptr)
    {
        return;
    }

    // Update the OpenGL texture with the current contents of the renderer's
    // framebuffer.
    this->glBindTexture(GL_TEXTURE_2D, FRAMEBUFFER_TEXTURE);
    this->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, fb->r.w, fb->r.h, 0, GL_BGRA, GL_UNSIGNED_BYTE, (u8*)fb->canvas);

    // Simple textured screen quad.
    glBegin(GL_TRIANGLES);
        glTexCoord2i(0, 0); glVertex2i(0, 0);
        glTexCoord2i(0, 1); glVertex2i(0, height());
        glTexCoord2i(1, 1); glVertex2i(width(), height());

        glTexCoord2i(1, 1); glVertex2i(width(), height());
        glTexCoord2i(1, 0); glVertex2i(width(), 0);
        glTexCoord2i(0, 0); glVertex2i(0, 0);
    glEnd();

    this->glFlush();

    return;
}

