#include "youtubeextractor.h"
#include <QtNetwork>
#include <QLocale>

/*
 * const QString ytInfoUrl = QStringLiteral("http://www.youtube.com/get_video_info?video_id=") + youtubeID + "&eurl="
                    + QString(QStringLiteral("https://youtube.googleapis.com/v/") + youtubeID).toUtf8();
 */
const QString FETCH_LINK = "https://www.youtube.com/get_video_info?video_id=%1%2&ps=default&eurl=&gl=US&hl=%3";

const int EXTRACT_ATTRIBUTE = 1234;
const int DOWNLOAD_ATTRIBUTE = EXTRACT_ATTRIBUTE + 1;

YouTubeExtractor::YouTubeExtractor(const QString &videoId, QObject *parent) :
    m_videoId(videoId),
    QObject(parent)
{
    setDefaults();

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinished(QNetworkReply*)));
}

YouTubeExtractor::YouTubeExtractor(const QUrl &requestUrl, QObject *parent) :
    QObject(parent)
{
    setRequestUrl(requestUrl);
    setDefaults();

    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinished(QNetworkReply*)));
}

void YouTubeExtractor::setDefaults()
{
    m_elFields << "embedded" << "detailpage" << "vevo" << "";
    m_preferredVideoQualities << Small << Medium << FLV_360
                              << FLV_480 << _3GP_240 <<_3GP_144
                              << _3GP_144 << MP4_720 << MP4_360
                              << MP4_480 << MP4_1080 << MP4_3072
                              << WEBM_360 << WEBM_720;
    m_supportedMedia << QMimeDatabase().mimeTypeForName("video/mp4");
}

void YouTubeExtractor::onFinished(QNetworkReply *reply)
{
    qDebug() << "YouTubeExtractor: Finished GET!";
    const int attribute = reply->request().attribute(QNetworkRequest::User).toInt();
    switch(attribute)
    {
    case EXTRACT_ATTRIBUTE:
    {
        const QByteArray response = reply->readAll();
        extractFromReply(QString(response));

        emit finished();
    }
        break;
    case DOWNLOAD_ATTRIBUTE:
        QFile file(m_thumbnailFilePath);
        if(!file.open(QIODevice::WriteOnly))
        {
            qDebug() << "YouTubeExtractor: Failed to write thumbnail to disk.";
            return;
        }

        emit thumbnailReady();
        break;
    }
}

QList<YouTubeExtractor::Quality> YouTubeExtractor::preferredVideoQualities()
{
    return m_preferredVideoQualities;
}

void YouTubeExtractor::setPreferredVideoQualities(const QList<Quality> &qualities)
{
    m_preferredVideoQualities = qualities;
}

void YouTubeExtractor::setSupportedMedia(const QList<QMimeType> &mimes)
{
    if(mimes.isEmpty())
        return;

    m_supportedMedia = mimes;
}

QList<QMimeType> YouTubeExtractor::supportedMedia() const
{
    return m_supportedMedia;
}

QUrl YouTubeExtractor::videoUrl(YouTubeExtractor::Quality quality) const
{
    switch(quality)
    {
    case Small:
        return m_videoUrls.Small;
        break;
    case Medium:
        return m_videoUrls.Medium;
        break;
    case High:
    case Default:
    case Standard:
        if(!m_videoUrls.MP4_3072.isEmpty())
            return m_videoUrls.MP4_3072;
        if(!m_videoUrls.MP4_1080.isEmpty())
            return m_videoUrls.MP4_1080;
        if(!m_videoUrls.MP4_720.isEmpty())
            return m_videoUrls.MP4_720;
        if(!m_videoUrls.MP4_480.isEmpty())
            return m_videoUrls.MP4_480;
        if(!m_videoUrls.MP4_360.isEmpty())
            return m_videoUrls.MP4_360;
        if(!m_videoUrls.WEBM_720.isEmpty())
            return m_videoUrls.WEBM_720;
        if(!m_videoUrls.WEBM_360.isEmpty())
            return m_videoUrls.WEBM_360;
        if(!m_videoUrls.FLV_480.isEmpty())
            return m_videoUrls.FLV_360;
        if(!m_videoUrls._3GP_240.isEmpty())
            return m_videoUrls._3GP_240;
        if(!m_videoUrls._3GP_144.isEmpty())
            return m_videoUrls._3GP_144;
        break;

    case FLV_360:
        return m_videoUrls.FLV_360;
        break;
    case FLV_480:
        return m_videoUrls.FLV_480;
        break;
    }

    return QUrl();
}

QUrl YouTubeExtractor::thumbnailUrl(Quality quality) const
{
    switch(quality)
    {
    case Small:
        return m_thumbnailUrls.Small;
        break;
    case Medium:
        return m_thumbnailUrls.Medium;
        break;
    case High:
        return m_thumbnailUrls.High;
        break;
    case Default:
        return m_thumbnailUrls.Default;
        break;
    case Standard:
        return m_thumbnailUrls.Standard;
        break;
    }

    return QUrl();
}

void YouTubeExtractor::setVideoId(const QString &videoId)
{
    if(videoId.trimmed().isEmpty())
        return;

    m_videoId = videoId;
}

void YouTubeExtractor::setRequestUrl(const QUrl &url)
{
    if(url.isEmpty())
        return;

    if(url.toString().startsWith("https://www.youtube.com/watch?v="))
        m_videoId = url.toString().remove("https://www.youtube.com/watch?v=", Qt::CaseInsensitive);
    else if(url.toString().startsWith("http://www.youtube.com/watch?v="))
        m_videoId = url.toString().remove("http://www.youtube.com/watch?v=", Qt::CaseInsensitive);
    else
        qDebug() << "YouTubeExtractor: Unable to parse request URL.";

    m_requestUrl = url;
}

void YouTubeExtractor::start()
{
    qDebug() << "YouTubeExtractor: Started!";
    QString elField = m_elFields.at(0);
    m_elFields.removeAll(0);

    if (elField.length() > 0)
        elField = "&el=" + elField;

    QString language = QLocale().languageToString(QLocale().language());
    const QString link = FETCH_LINK.arg(m_videoId, elField, language);

    QNetworkRequest request;
    request.setUrl(QUrl(link));
    request.setAttribute(QNetworkRequest::User, EXTRACT_ATTRIBUTE);
    m_manager->get(request);
}

void YouTubeExtractor::downloadThumbnail(const QString &filePath, YouTubeExtractor::Quality quality)
{
    m_thumbnailFilePath = filePath;
    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::User, DOWNLOAD_ATTRIBUTE);
    request.setUrl(thumbnailUrl(quality));
    m_manager->get(request);
}

bool YouTubeExtractor::isSupportedMedia(const QString &mime)
{
    /*QMimeDatabase().allMimeTypes()*/
    foreach(const QMimeType &t, m_supportedMedia)
    {
        if(t.name() == mime)
            return true;
    }

    return false;
}

//region Private
QMap<QString, QString> YouTubeExtractor::getMapFromQuery(const QString &query)
{
    QMap<QString, QString> map;

    QList<QByteArray> fields = query.toUtf8().split('&');

    foreach(const QByteArray &field, fields)
    {
        QList<QByteArray> pair = field.split('=');

        if (pair.length() == 2)
        {
            QString key = pair[0];
            // This string will still have percent signs in it, so you have to convert them to actual characters
            // with QByteArray::fromPercentEncoding().
            QString value = QByteArray::fromPercentEncoding(pair[1]);

            map.insert(key, value);
        }
    }

    return map;
}

void YouTubeExtractor::extractFromReply(const QString &html)
{
    QMap<QString, QString> video = getMapFromQuery(html.toUtf8());

    QUrl videoUrl;

    // Get the url encoded format stream map first
    if(video.contains("url_encoded_fmt_stream_map"))
    {
        QStringList streamQueries = video.value("url_encoded_fmt_stream_map").split(",");

        // Get adaptive format list (for some reason I am unaware of)
        QStringList adaptiveFormats = video.value("adaptive_fmts").split(",");
        // Append the two lists
        streamQueries.append(adaptiveFormats);


        QMap<int, QString> streamLinks;
        foreach(const QString &streamQuery, streamQueries)
        {
            QMap<QString, QString> streamMap = getMapFromQuery(streamQuery);

            // The value for the "type" key contains both the MIME data and the CODEC data.
            // To get the MIME data, split the value at the ";" and return the first value of the string list.
            QString type = streamMap.value("type").split(";").first();
            QString url = streamMap.value("url");

            if (!url.trimmed().isEmpty() && isSupportedMedia(type))
            {
                const QString signature = streamMap.value("sig").toUtf8();

                if (!signature.trimmed().isEmpty())
                {
                    url = url + "&signature=" + signature;
                }

                // Confused about this place
                QMap<QString, QString> urlMap = getMapFromQuery(url);
                if (urlMap.contains("signature"))
                {
                    streamLinks.insert(streamMap.value("itag").toInt(), url);
                }
            }
        }

        foreach(Quality quality, m_preferredVideoQualities)
        {
            if(streamLinks.value(quality) != 0)
            {
                const QString streamLink = streamLinks.value(quality);
                setVideoUrl(QUrl(streamLink.toUtf8()), quality);
                break;
            }
        }

        if(video.contains("iurlmq"))
            setThumbnailUrl(QUrl(video.value("iurlmq")), Medium);
        if(video.contains("iurlmq"))
            setThumbnailUrl(QUrl(video.value("iurlhq")), High);
        if(video.contains("iurl"))
            setThumbnailUrl(QUrl(video.value("iurl")), Default);
        if(video.contains("iurlsd"))
            setThumbnailUrl(QUrl(video.value("iurlsd")), Standard);
    }

    else
        qDebug() << "Failed to fetch video.";
}

void YouTubeExtractor::setVideoUrl(const QUrl &url, YouTubeExtractor::Quality quality)
{
    if(url.isEmpty())
        return;

    switch(quality)
    {
    case Small:
        m_videoUrls.Small = url;
        break;
    case Medium:
        m_videoUrls.Medium = url;
        break;
    case FLV_360:
        m_videoUrls.FLV_360 = url;
        break;
    case FLV_480:
        m_videoUrls.FLV_480 = url;
        break;
    case _3GP_144:
        m_videoUrls._3GP_144 = url;
        break;
    case _3GP_240:
        m_videoUrls._3GP_240 = url;
        break;
    case MP4_720:
        m_videoUrls.MP4_720 = url;
        break;
    case MP4_360:
        m_videoUrls.MP4_360 = url;
        break;
    case MP4_480:
        m_videoUrls.MP4_480 = url;
        break;
    case MP4_1080:
        m_videoUrls.MP4_1080 = url;
        break;
    case MP4_3072:
        m_videoUrls.MP4_3072 = url;
        break;
    case WEBM_360:
        m_videoUrls.WEBM_360 = url;
        break;
    case WEBM_720:
        m_videoUrls.WEBM_720 = url;
        break;
    }
}

void YouTubeExtractor::setThumbnailUrl(const QUrl &url, YouTubeExtractor::Quality quality)
{
    if(url.isEmpty())
        return;

    switch(quality)
    {
    case Small:
        m_thumbnailUrls.Small = url;
        break;
    case Medium:
        m_thumbnailUrls.Medium = url;
        break;
    case High:
        m_thumbnailUrls.High = url;
        break;
    case Default:
        m_thumbnailUrls.Default = url;
        break;
    case Standard:
        m_thumbnailUrls.Standard = url;
        break;
    }
}
