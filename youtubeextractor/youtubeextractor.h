#ifndef YOUTUBEEXTRACTOR_H
#define YOUTUBEEXTRACTOR_H

#include <QObject>
#include <QUrl>
#include <QMimeType>

class QNetworkAccessManager;
class QNetworkReply;

struct VideoUrls {
public:
    VideoUrls() {}
    VideoUrls(const QUrl &Small, const QUrl &Medium, const QUrl &FLV_360,
              const QUrl &FLV_480, const QUrl &_3GP_240, const QUrl &_3GP_144,
              const QUrl &MP4_720, const QUrl &MP4_360, const QUrl &MP4_480,
              const QUrl &MP4_1080, const QUrl &MP4_3072, const QUrl &WEBM_360,
              const QUrl &WEBM_720) :
        Small(Small), Medium(Medium),
        FLV_360(FLV_360), FLV_480(FLV_480), _3GP_240(_3GP_240),
        _3GP_144(_3GP_144), MP4_720(MP4_720),
        MP4_360(MP4_360), MP4_480(MP4_480), MP4_1080(MP4_1080),
        MP4_3072(MP4_3072), WEBM_360(WEBM_360), WEBM_720(WEBM_720) {}

    VideoUrls operator =(VideoUrls &other)
    {
        Small= other.Small;
        Medium = other.Medium;
        FLV_360 = other.FLV_360;
        FLV_480 = other.FLV_480;
        _3GP_240 = other._3GP_240;
        _3GP_144 = other._3GP_144;
        MP4_720 = other.MP4_720;
        MP4_360 = other.MP4_360;
        MP4_480 = other.MP4_480;
        MP4_1080 = other.MP4_1080;
        MP4_3072 = other.MP4_3072;
        WEBM_360 = other.WEBM_360;
        WEBM_720 = other.WEBM_720;

        return *this;
    }

    QUrl Small, Medium,
    FLV_360, FLV_480, _3GP_240, _3GP_144, MP4_720, MP4_360, MP4_480, MP4_1080, MP4_3072, WEBM_360, WEBM_720;
};

struct ThumbnailUrls {
    ThumbnailUrls() {}
    ThumbnailUrls(const QUrl &Small, const QUrl &Medium, const QUrl &High,
                           const QUrl &Default, const QUrl &Standard) :
        Small(Small), Medium(Medium), High(High),
        Default(Default), Standard(Standard) {}

    ThumbnailUrls operator =(ThumbnailUrls &other)
    {
        Small = other.Small;
        Medium = other.Medium;
        High = other.High;
        Default = other.Default;
        Standard = other.Standard;

        return *this;
    }

    QUrl Small, Medium, High, Default, Standard;
};

class YouTubeExtractor : public QObject
{
    Q_OBJECT
public:
    enum Quality {
        Small = 36,
        Medium = 0, // 18
        High = -1,
        Default = -2,
        Standard = -3,

        FLV_360 = 5,
        FLV_480 = 6,    // unsure
        _3GP_240 = 13,  // unsure
        _3GP_144 = 17,
        MP4_720 = 22,
        MP4_360 = 34,
        MP4_480 = 35,
        MP4_1080 = 37,
        MP4_3072 = 38,
        WEBM_360 = 43,
        WEBM_720 = 45
    };

    explicit YouTubeExtractor(const QString &videoId, QObject *parent = 0);
    explicit YouTubeExtractor(const QUrl &requestUrl, QObject *parent = 0);

    QList<Quality> preferredVideoQualities();
    void setPreferredVideoQualities(const QList<Quality> &preferredVideoQualities);

    bool isSupportedMedia(const QString &mime);
    void setSupportedMedia(const QList<QMimeType> &);
    QList<QMimeType> supportedMedia() const;

    QUrl videoUrl(Quality) const;
    QUrl thumbnailUrl(Quality) const;

    void setVideoId(const QString &videoId);
    void setRequestUrl(const QUrl &url);
public slots:
    void start();

    // Download thumbnail
    void downloadThumbnail(const QString &filePath, Quality = Default);
private slots:
    void onFinished(QNetworkReply *reply);
signals:
    void finished();
    void thumbnailReady();
private:
    QString m_videoId;
    QList<QString> m_elFields;
    QNetworkAccessManager *m_manager;
    QList<Quality> m_preferredVideoQualities;
    VideoUrls m_videoUrls;
    ThumbnailUrls m_thumbnailUrls;
    QList<QMimeType> m_supportedMedia;
    QString m_thumbnailFilePath;
    QUrl m_requestUrl;

    void setDefaults();
    QMap<QString, QString> getMapFromQuery(const QString &query);

    void extractFromReply(const QString &html);

    void setVideoUrl(const QUrl &url, Quality);
    void setThumbnailUrl(const QUrl &url, Quality);
};

#endif // YOUTUBEEXTRACTOR_H

//Video formats     Resolution      Itag
//FLV               360p            5
//FLV               480p?           6
//3GP               240?            13
//3GP               144p            17
//MP4               480p            18
//MP4               720p            22
//MP4               360p            34
//MP4               480p            35
//MP4               1080p           37
//MP4               3072p           38
//WEBM              360p            43
//WEBM              720p            45

//Key               Value
//url               stream/download URL
//type              mime type
//quality           small, medium, large, hd720, hd1080
//itag              number indicating the video type and quality
//fallback_host     alternative host name
