#ifndef IMAGEHOLDER_HPP
#define IMAGEHOLDER_HPP

#include "fileiterator.hpp"
#include "imagestates.hpp"
#include "imageprocessing.hpp"

#include <QObject>
#include <QImage>

#include <memory>
#include <functional>

class ImageHolder : public QObject
{
    Q_OBJECT
public:
    explicit ImageHolder() : QObject() {}

    inline std::shared_ptr<const QImage> NextImageState() { return imageStates_.NextState(); }
    inline std::shared_ptr<const QImage> PrevImageState() { return imageStates_.PrevState(); }
    inline std::shared_ptr<const QImage> RevertImage()    { return imageStates_.Revert(); }

signals:
    void ImageLoaded(std::shared_ptr<const QImage>);
    void ProcessingDone(std::shared_ptr<const QImage>);
    void HistogramDone(HistRGB);

public slots:
    void LoadImageFile(QString filename);
    void LoadPreviousImageFile();
    void LoadNextImageFile();

    void StartImageProcessing(std::function<void(QImage&)> processingFunction);
    void StartHistogram();

private:
    void LoadImage(const QString& filename);

private:
    ImageStates imageStates_;
    FileIterator fileIterator_;
};

#endif // IMAGEHOLDER_HPP
