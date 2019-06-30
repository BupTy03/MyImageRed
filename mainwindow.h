#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imageprocessor.h"
#include "inputmatrix.h"
#include "matrix.h"
#include "histogram.h"

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

class ImageProc;

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
    void on_GammaBtn_toggled(bool checked);
    void on_GammaDSpinBox_1_valueChanged(double arg1);
    void on_GammaDSpinBox_2_valueChanged(double arg1);
    void on_GammaOk_clicked();
    void on_GBOkBtn_clicked();
    void on_MedianBtn_toggled(bool checked);
    void on_MedianOkBtn_clicked();
    void on_MedianSBox_valueChanged(int arg1);
    void on_CustomBtn_clicked();
    void on_ErosionOkBtn_clicked();
    void on_IncreaseOkBtn_clicked();
    void on_Quit_clicked();
    void on_HistogramBtn_clicked();
    void on_RotateLeftBtn_clicked();
    void on_RotateRightBtn_clicked();
    void on_HMirroredBtn_clicked();
    void on_VMirroredBtn_clicked();
    void on_PrevBtn_clicked();
    void on_NextBtn_clicked();
    void on_QuickSaveBtn_clicked();

private:
    Ui::MainWindow *ui;
    QScopedPointer<QImage> MyIMG;
    QScopedPointer<QImage> TmpIMG;
    QScopedPointer<QStringList> CurrFileList;
    QStringList::iterator CurrFileIt;

    InputMatrix* inMtx;
    QScopedPointer<ImageProcessor> imgProc;
    QThread* MyThread;


    virtual void resizeEvent(QResizeEvent*) override;

    void update_pixmap();
    bool loadImage(const QString& str);
    void EnableAll(bool flag);
    void StartProcess();

private slots:
    void CustomMatrix();
    void on_IncreaseRadioBtn_toggled(bool checked);
    void on_ErosionRadioBtn_toggled(bool checked);
    void on_IncreaseSpinBox_valueChanged(int arg1);
    void on_ErosionSpinBox_valueChanged(int arg1);
    void ProcIsDone();

signals:
    void LinCorrStart(QImage*);
    void GrayWorldStart(QImage*);
    void GammaStart(QImage*, double, double);
    void GBStart(QImage*);
    void MedianStart(QImage*, const int);
    void CustomStart(QImage*, vector<double>*);
    void ErosionStart(QImage*, const int);
    void IncreaseStart(QImage*, const int);
    void RotateLeftStart(QImage*);
    void RotateRightStart(QImage*);
    void HMirrorStart(QImage*);
    void VMirrorStart(QImage*);
};

#endif // MAINWINDOW_H
