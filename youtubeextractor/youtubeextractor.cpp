#include "youtubeextractor.h"
#include <QtNetwork>
#include <QLocale>

// First
// /(?:youtube\\.com\\/\\S*(?:(?:\\/e(?:mbed))?\\/|watch\\/?\\?(?:\\S*?&?v\\=))|youtu\\.be\\/)([a-zA-Z0-9_-]{6,11})

// Second
//http(?:s?):\\/\\/(?:www\\.)?youtu(?:be\\.com\\/watch\\?v=|\\.be\\/)([\\w\\-\\_]*)(&(amp;)?‌​[\\w\\?‌​=]*)?x

// Works with URLs that have http or https in front
//const QString URL_PATTERN = "http(?:s?):\\/\\/(?:www\\.)(?:youtube\\.com\\/\\S*(?:(?:\\/e(?:mbed))?\\/|watch\\/?\\?(?:\\S*?&?v\\=))|youtu\\.be\\/)([a-zA-Z0-9_-]{6,11})";


// Link used to fetch the RTSP URL; The %[number] are placeholders
const QString FETCH_LINK = "https://www.youtube.com/get_video_info?video_id=%1%2&ps=default&eurl=&gl=US&hl=%3";

// Works after "http:/", "https:/" and "www." is removed from the URL
const QString URL_PATTERN = "/(?:youtube\\.com\\/\\S*(?:(?:\\/e(?:mbed))?\\/|watch\\/?\\?(?:\\S*?&?v\\=))|youtu\\.be\\/)([a-zA-Z0-9_-]{6,11})";

YouTubeExtractor::YouTubeExtractor(QObject *parent) :
    QObject(parent)
{
    setDefaults();
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinished(QNetworkReply*)));
}

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

    m_supportedMedia << QMimeDatabase().mimeTypeForName("video/mp4")
                     << QMimeDatabase().mimeTypeForName("video/webm")
                     << QMimeDatabase().mimeTypeForName("video/3gp");
}

void YouTubeExtractor::onFinished(QNetworkReply *reply)
{
    const Attribute attribute = (Attribute) reply->request().attribute(QNetworkRequest::User).toInt();

    try {
        switch(attribute)
        {
        case ExtractAttribute:
            if(reply->error() != QNetworkReply::NoError)
            {
                throw YouTubeExtractorException(YouTubeExtractorError::NetworkError,
                                                reply->errorString());
            }
            else
            {
                const QByteArray response = reply->readAll();
                extractFromReply(QString(response));
                emit finished();
            }
            break;
        case DownloadAttribute:
            if(reply->error() != QNetworkReply::NoError)
            {
                throw YouTubeExtractorException(YouTubeExtractorError::NetworkError,
                                                reply->errorString());
            }
            else
            {
                QFile file(m_thumbnailFilePath);
                if(!file.open(QIODevice::WriteOnly))
                {
                    throw YouTubeExtractorException(YouTubeExtractorError::FileError,
                                                    file.errorString());
                }

                emit thumbnailReady();
            }
            break;
        }
    }
    catch(YouTubeExtractorException &e)
    {
        qDebug() << "YouTubeExtractor:" << e.text();
        setLastError(YouTubeExtractorError(e.code(), e.text()));

        if(attribute == ExtractAttribute)
            emit finished();
        else if(attribute == DownloadAttribute)
            emit thumbnailReady();
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

QString YouTubeExtractor::videoId() const
{
    return m_videoId;
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
    case Any:
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
    case Any:
        if(m_thumbnailUrls.Standard.isEmpty())
            return m_thumbnailUrls.Default;
        if(m_thumbnailUrls.Default.isEmpty())
            return m_thumbnailUrls.High;
        if(m_thumbnailUrls.High.isEmpty())
            return m_thumbnailUrls.Medium;
        if(m_thumbnailUrls.Medium.isEmpty())
            return m_thumbnailUrls.Small;
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
    try {
        if(url.isEmpty())
            throw YouTubeExtractorException(YouTubeExtractorError::UrlError, tr("The URL provided is empty."));

        const QString urlString = url.toString().remove("http:/").remove("https:/").remove("www.");

        QRegularExpression re(URL_PATTERN);
        if(!re.isValid())
            throw YouTubeExtractorException(YouTubeExtractorError::RegexError, re.errorString());

        QRegularExpressionMatch match = re.match(urlString);
        if (match.hasMatch())
            m_videoId = match.captured(1);
        else
            throw YouTubeExtractorException(YouTubeExtractorError::RegexError, tr("Unable to parse request URL."));

        m_requestUrl = url;
    }
    catch(YouTubeExtractorException &e)
    {
        qDebug() << "YouTubeExtractor:" << e.what();
        setLastError(YouTubeExtractorError(e.code(), e.text()));
    }
}

YouTubeExtractorError YouTubeExtractor::lastError() const
{
    return m_error;
}

void YouTubeExtractor::start()
{
    try {
        if(m_videoId.trimmed().isEmpty())
            throw YouTubeExtractorException(YouTubeExtractorError::IdError, tr("No video ID provided."));
        else
        {
            QString elField = m_elFields.at(0);
            m_elFields.removeAll(0);

            if (elField.length() > 0)
                elField = "&el=" + elField;

            QString language = QLocale().languageToString(QLocale().language());
            const QString link = FETCH_LINK.arg(m_videoId, elField, language);

            QNetworkRequest request;
            request.setUrl(QUrl(link));
            request.setAttribute(QNetworkRequest::User, ExtractAttribute);
            m_manager->get(request);
        }
    }
    catch(YouTubeExtractorException &e)
    {
        qDebug() << "YouTubeExtractor: " << e.text();
        setLastError(YouTubeExtractorError(e.code(), e.text()));
        emit finished();
    }
}

void YouTubeExtractor::downloadThumbnail(const QString &filePath, YouTubeExtractor::Quality quality)
{
    try {
        if(thumbnailUrl(Any).isEmpty())
            throw YouTubeExtractorException(YouTubeExtractorError::UrlError, tr("No thumbnail URL available."));
        else if(filePath.trimmed().isEmpty())
            throw YouTubeExtractorException(YouTubeExtractorError::FileError, tr("The file path provided is empty."));
        else
        {
            m_thumbnailFilePath = filePath;
            QNetworkRequest request;
            request.setAttribute(QNetworkRequest::User, DownloadAttribute);
            request.setUrl(thumbnailUrl(quality));
            m_manager->get(request);
        }
    }
    catch(YouTubeExtractorException &e)
    {
        qDebug() << "YouTubeExtractor: " << e.what();
        setLastError(YouTubeExtractorError(e.code(), e.text()));
        emit thumbnailReady();
    }
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
        throw YouTubeExtractorException(YouTubeExtractorError::ParseError, QString(video.value("reason")).replace("+", " "));
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

void YouTubeExtractor::setLastError(YouTubeExtractorError e)
{
    m_error = e;
}
