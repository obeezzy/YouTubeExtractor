#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtAV>
#include <QtAVWidgets>

class YouTubeExtractor;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    YouTubeExtractor *m_extractor;
    QtAV::AVPlayer *m_player;
    QtAV::GLWidgetRenderer2 *m_renderer;
};

#endif // MAINWINDOW_H
