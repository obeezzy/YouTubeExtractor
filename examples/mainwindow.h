#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtAV>
#include <QtAVWidgets>

class VlcInstance;
class VlcMediaPlayer;
class VlcMedia;
class VlcWidgetVideo;
class QNetworkAccessManager;
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
    QNetworkAccessManager *manager;
    YouTubeExtractor *extractor;
    QtAV::AVPlayer *mPlayer;
    QtAV::GLWidgetRenderer2 *mRenderer;

    VlcInstance *mInstance;
    VlcMediaPlayer *mMediaPlayer;
    VlcMedia *mMedia;
    VlcWidgetVideo *mVideoWidget;
};

#endif // MAINWINDOW_H
