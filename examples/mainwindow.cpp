#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "youtubeextractor/youtubeextractor.h"
#include <QtAV>
#include <QtAVWidgets>
#include <QDebug>
#include <QUrlQuery>
#include <QMimeDatabase>

const QString youtubeLink = "https://www.youtube.com/watch?v=bYo885OoWtY";
const QString youtubeID = "bYo885OoWtY";

using namespace QtAV;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Init renderer;
    m_renderer = new GLWidgetRenderer2(this);
    setCentralWidget(m_renderer);

    // Init player
    m_player = new AVPlayer(this);
    m_player->setRenderer(m_renderer);

    // Init extractor
    m_extractor = new YouTubeExtractor(QUrl(youtubeLink), this);
    connect(m_extractor, &YouTubeExtractor::finished, [this]()
    {
        if(!m_extractor->lastError().isValid())
        {
            m_player->setFile(m_extractor->videoUrl(YouTubeExtractor::Any).toString());
            m_player->play();
        }
        else
        {
            qDebug() << "YouTubeExtractor:" << m_extractor->lastError().text();
        }
    });

    // Start extracting
    m_extractor->start();
}

MainWindow::~MainWindow()
{

}
