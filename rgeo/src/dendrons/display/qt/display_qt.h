/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 */

#ifndef D_WINDOW_H
#define D_WINDOW_H

#include <QWidget>
#include "../../../core/types.h"

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(const uint width, const uint height);
    ~Window();

    QPointF CursorPosition();

private:
    void closeEvent(QCloseEvent*);
    bool eventFilter(QObject*, QEvent *event);
};

#endif
