#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imageprocessor.hpp"
#include "inputmatrixdialog.hpp"
#include "matrix.hpp"
#include "histogram.hpp"

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
    void on_SaveBtn_clicked();
    void on_LoadBtn_clicked();
    void on_CancelBtn_clicked();
    void on_LinCorrBtn_clicked();
    void on_GrayWorldBtn_clicked();
    void on_GBOkBtn_clicked();
    void on_CustomBtn_clicked();
    void on_Quit_clicked();
    void on_HistogramBtn_clicked();
    void on_RotateLeftBtn_clicked();
    void on_RotateRightBtn_clicked();
    void on_HMirroredBtn_clicked();
    void on_VMirroredBtn_clicked();
    void on_PrevBtn_clicked();
    void on_NextBtn_clicked();
    void on_QuickSaveBtn_clicked();

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent*) override { updatePixmap(); }

    void updatePixmap();
    bool loadImage(const QString& str);
    void EnableAll(bool flag);
    void StartProcess();

private slots:
    void CustomMatrix();
    void ProcIsDone();

signals:
    void LinCorrStart(QImage*);
    void GrayWorldStart(QImage*);
    void GammaStart(QImage*, double, double);
    void GBStart(QImage*);
    void MedianStart(QImage*, const int);
    void CustomStart(QImage*, const std::vector<double>*);
    void ErosionStart(QImage*, const int);
    void IncreaseStart(QImage*, const int);
    void RotateLeftStart(QImage*);
    void RotateRightStart(QImage*);
    void HMirrorStart(QImage*);
    void VMirrorStart(QImage*);

private:
    Ui::MainWindow *ui{nullptr};
    std::unique_ptr<QImage> myIMG_;
    std::unique_ptr<QImage> tmpIMG_;
    std::unique_ptr<QStringList> currFileList_;
    QStringList::iterator currFileIt_;

    InputMatrixDialog* inMtx_;
    std::unique_ptr<ImageProcessor> imgProc_;
    std::unique_ptr<QThread, QThreadDeleter> myThread_;
};

#endif // MAINWINDOW_H
