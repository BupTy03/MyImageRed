#ifndef IMAGEHOLDER_HPP
#define IMAGEHOLDER_HPP

#include "fileiterator.hpp"
#include "imageprocessing.hpp"

#include <QObject>
#include <QImage>

#include <memory>
#include <functional>

class ImageHolder : public QObject
{
    Q_OBJECT
public:
    explicit ImageHolder(QObject *parent = nullptr);

signals:
    void Update(std::shared_ptr<QImage>);
    void ImageLoaded(std::shared_ptr<QImage>);
    void ProcessingDone(std::shared_ptr<QImage>);
    void HistogramDone(HistRGB);

public slots:
    void OnRequestUpdate() { Update(img_); }

    void LoadImageFile(QString filename);
    void LoadPreviousImageFile();
    void LoadNextImageFile();
    void RevertImage();
    void StartImageProcessing(std::function<void(QImage&)> processingFunction);

    void StartHistogram();

private:
    void LoadImage(const QString& filename);

private:
    std::shared_ptr<QImage> img_;
    std::shared_ptr<QImage> tmpIMG_;
    FileIterator fileIterator_;
};

#endif // IMAGEHOLDER_HPP
