#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imageprocessor.hpp"
#include "inputmatrixdialog.hpp"
#include "matrix.hpp"
#include "histogram.hpp"
#include "fileiterator.hpp"
#include "imageholder.hpp"

#include <QMainWindow>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QRgb>
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsView>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QThread>
#include <QDir>
#include <QFileInfo>

#include <array>
#include <memory>
#include <utility>

using namespace std;

namespace Ui {
class MainWindow;
}

class ImageProcessor;

struct QThreadDeleter {
    void operator()(QThread* pThread) const {
        pThread->quit();
        pThread->wait();
        delete pThread;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow() override;

private slots:
    void OnActionSave();
    void OnActionLoad();
    void OnActionCancel();
    void OnActionLinearCorrection();
    void OnActionGrayWorld();
    void OnActionGaussBlur();
    void OnActionCustom();
    void OnActionHistogram();
    void OnButtonHMirrored();
    void OnButtonVMirrored();
    void OnActionQuickSave();

    void OnCustomMatrix();

    void OnButtonPrev();
    void OnButtonNext();
    void OnButtonRotateLeft();
    void OnButtonRotateRight();


    void ProcIsDone(std::shared_ptr<QImage> pImage);

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent*) override { UpdatePixmap(); }

private:
    void UpdatePixmap();
    bool LoadImage(const QString& str);
    void EnableAll(bool flag);
    void StartProcess();

signals:
    void StartProcessing(std::shared_ptr<QImage>);

    void StartLoadingImage(QString);

    void ProcessImage(std::function<void(QImage&)>);

    void LinearCorrectionStart(const QImage&);
    void GrayWorldStart(const QImage&);
    void GammaStart(const QImage&, double, double);
    void GaussBlurStart(const QImage&);
    void MedianStart(const QImage&, int);
    void CustomStart(const QImage&, const std::vector<double>&);
    void ErosionStart(const QImage&, int);
    void IncreaseStart(const QImage&, int);
    void RotateLeftStart(const QImage&);
    void RotateRightStart(const QImage&);
    void HMirrorStart(const QImage&);
    void VMirrorStart(const QImage&);

private:
    Ui::MainWindow *ui{nullptr};

    std::shared_ptr<QImage> myIMG_;
    std::shared_ptr<QImage> tmpIMG_;

    FileIterator fileIterator_;

    std::unique_ptr<ImageHolder> imgHolder_;

    InputMatrixDialog* inputMatrixDialog_;
    std::unique_ptr<ImageProcessor> imageProcessor_;
    std::unique_ptr<QThread, QThreadDeleter> myThread_;
};

#endif // MAINWINDOW_H
