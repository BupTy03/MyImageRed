#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imageprocessor.hpp"
#include "inputmatrixdialog.hpp"
#include "matrix.hpp"
#include "histogram.hpp"
#include "fileiterator.hpp"

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
    void OnActionRotateLeft();
    void OnActionRotateRight();
    void OnActionHMirrored();
    void OnActionVMirrored();
    void OnActionQuickSave();

    void OnCustomMatrix();

    void OnButtonPrev();
    void OnButtonNext();


    void ProcIsDone();

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
    void LinearCorrectionStart(QImage*);
    void GrayWorldStart(QImage*);
    void GammaStart(QImage*, double, double);
    void GaussBlurStart(QImage*);
    void MedianStart(QImage*, int);
    void CustomStart(QImage*, const std::vector<double>*);
    void ErosionStart(QImage*, int);
    void IncreaseStart(QImage*, int);
    void RotateLeftStart(QImage*);
    void RotateRightStart(QImage*);
    void HMirrorStart(QImage*);
    void VMirrorStart(QImage*);

private:
    Ui::MainWindow *ui{nullptr};

    QImage myIMG_;
    QImage tmpIMG_;

    FileIterator fileIterator_;

    std::unique_ptr<QStringList> currFileList_;
    QStringList::iterator currFileIt_;

    InputMatrixDialog* inputMatrixDialog_;
    std::unique_ptr<ImageProcessor> imageProcessor_;
    std::unique_ptr<QThread, QThreadDeleter> myThread_;
};

#endif // MAINWINDOW_H
