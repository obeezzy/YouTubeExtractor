# YouTubeExtractor
A YouTube video URL extractor for Qt

## How to use
The easiest way to use this library in your project is to copy the "youtubeextractor" folder to your project and add the **.h** and **.cpp* file to the **.pro** file of your project.

Using this library in code is straightforward. Simply create an instance of the class YouTubeExtractor and pass in the video ID or the YouTube URL of choice into the YouTubeExtractor constructor.

    // Assuming YouTubeExtractor has been defined in the .h file
    extractor = new YouTubeExtractor(QUrl("https://www.youtube.com/watch?v=tPEE9ZwTmy0"));
    // or extractor = new YouTubeExtractor("tPEE9ZwTmy0");
    
    // Using C++11
    // Once the YouTubeExtractor instance has fetched from the YouTube URL, run this slot.
    connect(extractor, &YouTubeExtractor::finished, []()
    {
      // Are there any errors?
      if(extractor->lastError().isValid())
        qDebug() << "YouTubeExtractor error: " << extractor->lastError().text();
      
      // If no errors, display the YouTube URL. The quality has been set to "Any", which would basically select
      // any quality available.
      else
        qDebug() << "Show me my YouTube URL: " << extractor->videoUrl(YouTubeExtractor::Any);
    });
    
    // Start the process!
    extractor->start();

## Todo
- Structure the current example in a more lucid way
- Add current example as a [QtAV](https://github.com/wang-bin/QtAV) example
- Create an example for use with [VLC-Qt](https://github.com/vlc-qt/vlc-qt)
- Provide an example that uses the library raw (without GUI)
- Provide example for downloading YouTube videos
