#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "youtubeextractor/youtubeextractor.h"
#include <QtAV>
#include <QtAVWidgets>
#include <QDebug>
#include <QUrlQuery>

const QString youtubeLink = "https://www.youtube.com/watch?v=bYo885OoWtY";
const QString youtubeID = "bYo885OoWtY";

using namespace QtAV;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Init renderer;
    mRenderer = new GLWidgetRenderer2(this);
    setCentralWidget(mRenderer);

    // Init player
    mPlayer = new AVPlayer(this);
    mPlayer->setRenderer(mRenderer);

    // Init extractor
    extractor = new YouTubeExtractor(QUrl(youtubeLink), this);
    connect(extractor, &YouTubeExtractor::finished, [this]()
    {
        mPlayer->setFile(extractor->videoUrl(YouTubeExtractor::High).toString());
        mPlayer->play();
    });

    // Start extracting
    extractor->start();
}

MainWindow::~MainWindow()
{

}
