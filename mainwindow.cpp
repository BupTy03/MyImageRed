#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "myimageproc.h"
#include "imageprocessor.h"
#include "inputmatrixdialog.h"

#include <algorithm>

#include <QLineSeries>
#include <QChartView>
#include <QValueAxis>
#include <QFileInfoList>

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , myIMG_{std::make_unique<QImage>()}
    , tmpIMG_{std::make_unique<QImage>()}
    , inMtx_{new InputMatrixDialog(this)}
    , imgProc_{std::make_unique<ImageProcessor>()}
    , myThread_{new QThread}
{
    ui->setupUi(this);
    setWindowTitle("Обработка изображений");

    imgProc_->moveToThread(myThread_.get());
    myThread_->start();

    ui->SaveBtn->setDisabled(true);
    ui->QuickSaveBtn->setDisabled(true);
    ui->QuickSaveBtn->setIcon(QIcon(":Save"));
    ui->CancelBtn->setDisabled(true);

    ui->RotateLeftBtn->setDisabled(true);
    ui->RotateLeftBtn->setIcon(QIcon(":rotateLeft"));
    connect(this, SIGNAL(RotateLeftStart(QImage*)), imgProc_.get(), SLOT(RotateLeftGo(QImage*)));

    ui->RotateRightBtn->setDisabled(true);
    ui->RotateRightBtn->setIcon(QIcon(":rotateRight"));
    connect(this, SIGNAL(RotateRightStart(QImage*)), imgProc_.get(), SLOT(RotateRightGo(QImage*)));

    ui->HMirroredBtn->setDisabled(true);
    ui->HMirroredBtn->setIcon(QIcon(":HMirror"));
    connect(this, SIGNAL(HMirrorStart(QImage*)), imgProc_.get(), SLOT(HMirrorGo(QImage*)));

    ui->VMirroredBtn->setDisabled(true);
    ui->VMirroredBtn->setIcon(QIcon(":VMirror"));
    connect(this, SIGNAL(VMirrorStart(QImage*)), imgProc_.get(), SLOT(VMirrorGo(QImage*)));

    ui->PrevBtn->setDisabled(true);
    ui->PrevBtn->setIcon(QIcon(":Prev"));

    ui->NextBtn->setDisabled(true);
    ui->NextBtn->setIcon(QIcon(":Next"));

    ui->LinCorrBtn->setDisabled(true);
    connect(this, SIGNAL(LinCorrStart(QImage*)), imgProc_.get(), SLOT(LinearCorrGo(QImage*)));

    ui->GrayWorldBtn->setDisabled(true);
    connect(this, SIGNAL(GrayWorldStart(QImage*)), imgProc_.get(), SLOT(GrayWorldGo(QImage*)));

    ui->GammaBtn->setDisabled(true);
    ui->GammaLabel_1->hide();
    ui->GammaDSpinBox_1->setMinimum(1);
    ui->GammaDSpinBox_1->hide();
    ui->GammaLabel_2->hide();
    ui->GammaDSpinBox_2->hide();
    ui->GammaDSpinBox_2->setMinimum(1);
    ui->GammaOk->setDisabled(true);
    ui->GammaOk->hide();
    connect(this, SIGNAL(GammaStart(QImage*,double,double)), imgProc_.get(), SLOT(GammaFuncGo(QImage*,double,double)));

    ui->GBOkBtn->setDisabled(true);
    connect(this, SIGNAL(GBStart(QImage*)), imgProc_.get(), SLOT(GaussBlurGo(QImage*)));

    ui->MedianBtn->setDisabled(true);
    ui->MedianLabel_1->hide();
    ui->MedianSBox->hide();
    ui->MedianSBox->setRange(3, 63);
    ui->MedianSBox->setSingleStep(2);
    ui->MedianOkBtn->hide();
    connect(this, SIGNAL(MedianStart(QImage*,int)), imgProc_.get(), SLOT(MedianFilterGo(QImage*,int)));

    ui->CustomBtn->setDisabled(true);
    connect(this, SIGNAL(CustomStart(QImage*, vector<double>*)), imgProc_.get(), SLOT(CustomFilterGo(QImage*, vector<double>*)));

    ui->ErosionRadioBtn->setDisabled(true);
    ui->ErosionSpinBox->setRange(3, 63);
    ui->ErosionSpinBox->setSingleStep(2);
    ui->ErosionLabel->hide();
    ui->ErosionSpinBox->hide();
    ui->ErosionOkBtn->hide();
    connect(this, SIGNAL(ErosionStart(QImage*,int)), imgProc_.get(), SLOT(ErosionGo(QImage*,int)));

    ui->IncreaseRadioBtn->setDisabled(true);
    ui->IncreaseSpinBox->setRange(3, 63);
    ui->IncreaseSpinBox->setSingleStep(2);
    ui->IncreaseLabel->hide();
    ui->IncreaseSpinBox->hide();
    ui->IncreaseOkBtn->hide();
    connect(this, SIGNAL(IncreaseStart(QImage*,int)), imgProc_.get(), SLOT(IncreaseGo(QImage*,int)));

    connect(inMtx_, SIGNAL(valuesChecked()), this, SLOT(CustomMatrix()));

    connect(imgProc_.get(), SIGNAL(isDone()), this, SLOT(ProcIsDone()));

    ui->HistogramBtn->setDisabled(true);

    ui->ProgressLabel->setText("");
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::resizeEvent(QResizeEvent*)
{
    update_pixmap();
}

void MainWindow::update_pixmap()
{
    if(myIMG_->isNull())
        return;

    QPixmap pm = QPixmap::fromImage(*myIMG_);
    ui->label->setPixmap(pm.scaled(ui->label->width(), ui->label->height(), Qt::KeepAspectRatio));
}

bool MainWindow::loadImage(const QString &str)
{
    if(!myIMG_->load(str))
        return false;

    if(QImage::Format::Format_RGB32 != myIMG_->format()) {
        *myIMG_ = myIMG_->convertToFormat(QImage::Format_RGB32);
    }
    *tmpIMG_ = *myIMG_;
    update_pixmap();

    return true;
}

void MainWindow::EnableAll(bool flag)
{
    ui->CancelBtn->setEnabled(flag);
    ui->CustomBtn->setEnabled(flag);
    ui->ErosionRadioBtn->setEnabled(flag);
    ui->ErosionOkBtn->setEnabled(flag);
    ui->GammaBtn->setEnabled(flag);
    ui->GammaOk->setEnabled(flag);
    ui->GBOkBtn->setEnabled(flag);
    ui->GrayWorldBtn->setEnabled(flag);
    ui->IncreaseRadioBtn->setEnabled(flag);
    ui->IncreaseOkBtn->setEnabled(flag);
    ui->LinCorrBtn->setEnabled(flag);
    ui->LoadBtn->setEnabled(flag);
    ui->MedianBtn->setEnabled(flag);
    ui->MedianOkBtn->setEnabled(flag);
    ui->Quit->setEnabled(flag);
    ui->SaveBtn->setEnabled(flag);
    ui->HistogramBtn->setEnabled(flag);
    ui->HMirroredBtn->setEnabled(flag);
    ui->NextBtn->setEnabled(flag);
    ui->PrevBtn->setEnabled(flag);
    ui->RotateLeftBtn->setEnabled(flag);
    ui->RotateRightBtn->setEnabled(flag);
    ui->VMirroredBtn->setEnabled(flag);
    ui->QuickSaveBtn->setEnabled(flag);
}

void MainWindow::StartProcess()
{
    ui->ProgressLabel->setText("Обработка...");
    EnableAll(false);
}

void MainWindow::on_SaveBtn_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить как"), QDir::currentPath(), tr("*.jpg *.jpeg *.png *.bmp"));

    if(fileName.isEmpty())
        return;

    myIMG_->save(fileName);
    ui->ProgressLabel->setText("Сохранено");
}

void MainWindow::on_LoadBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть файл"), "/", tr("*.jpg *.jpeg *.png *.bmp"));

    loadImage(fileName);

    QDir currDir(fileName);
    currDir.cdUp();

    currFileList_ = std::make_unique<QStringList>(currDir.entryList(QDir::Files));

    std::for_each(currFileList_->begin(), currFileList_->end(), [&currDir](QString& item){
        item = currDir.path() + '/' + item;
    });

    currFileIt_ = std::find(currFileList_->begin(), currFileList_->end(), fileName);

    ui->GammaBtn->setEnabled(true);
    ui->MedianBtn->setEnabled(true);
    ui->ErosionRadioBtn->setEnabled(true);
    ui->IncreaseRadioBtn->setEnabled(true);

    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::on_CancelBtn_clicked()
{
    *myIMG_ = *tmpIMG_;
    update_pixmap();
    ui->CancelBtn->setDisabled(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::on_LinCorrBtn_clicked()
{
    StartProcess();
    emit LinCorrStart(myIMG_.get());
}

void MainWindow::on_GrayWorldBtn_clicked()
{
    StartProcess();
    emit GrayWorldStart(myIMG_.get());
}

void MainWindow::on_GammaBtn_toggled(bool checked)
{
    if(checked)
    {
        ui->GammaLabel_1->show();
        ui->GammaDSpinBox_1->show();
        ui->GammaLabel_2->show();
        ui->GammaDSpinBox_2->show();
        ui->GammaOk->setDisabled(true);
        ui->GammaOk->show();
    }
    else{
        ui->GammaLabel_1->hide();
        ui->GammaDSpinBox_1->hide();
        ui->GammaLabel_2->show();
        ui->GammaDSpinBox_2->hide();
        ui->GammaOk->setDisabled(true);
        ui->GammaOk->hide();
    }
}

void MainWindow::on_GammaDSpinBox_1_valueChanged(double arg1)
{
     ui->GammaOk->setEnabled(arg1 != 1.0);
}

void MainWindow::on_GammaDSpinBox_2_valueChanged(double arg1)
{
    ui->GammaOk->setEnabled(arg1 != 1.0);
}

void MainWindow::on_GammaOk_clicked()
{
    StartProcess();
    emit GammaStart(myIMG_.get(), ui->GammaDSpinBox_1->value(), ui->GammaDSpinBox_2->value());
}

void MainWindow::on_GBOkBtn_clicked()
{
    StartProcess();
    emit GBStart(myIMG_.get());
}

void MainWindow::on_MedianBtn_toggled(bool checked)
{
    if(checked)
    {
        ui->MedianLabel_1->show();
        ui->MedianSBox->show();
        ui->MedianOkBtn->show();
    }
    else{
        ui->MedianLabel_1->hide();
        ui->MedianSBox->hide();
        ui->MedianOkBtn->hide();
    }
}

void MainWindow::ProcIsDone()
{
    EnableAll(true);
    update_pixmap();
    ui->ProgressLabel->setText("Готово");
}

void MainWindow::on_MedianOkBtn_clicked()
{
    StartProcess();
    emit MedianStart(myIMG_.get(), ui->MedianSBox->value());
}

void MainWindow::on_MedianSBox_valueChanged(int arg1)
{
    ui->MedianOkBtn->setEnabled(arg1 % 2);
}

void MainWindow::CustomMatrix()
{
    StartProcess();
    emit CustomStart(myIMG_.get(), inMtx_->getValuesPtr());
}

void MainWindow::on_CustomBtn_clicked()
{
    inMtx_->show();
}

void MainWindow::on_Quit_clicked()
{
    auto reply = QMessageBox::question(this, "Выход", "Вы хотите завершить приложение ?",
                                       QMessageBox::Yes | QMessageBox::No);

    if(reply == QMessageBox::Yes)
        QApplication::quit();
}

void MainWindow::on_ErosionOkBtn_clicked()
{
    StartProcess();
    emit ErosionStart(myIMG_.get(), ui->ErosionSpinBox->value());
}

void MainWindow::on_ErosionRadioBtn_toggled(bool checked)
{
    if(checked)
    {
        ui->ErosionLabel->show();
        ui->ErosionSpinBox->show();
        ui->ErosionOkBtn->show();
    }
    else{
        ui->ErosionLabel->hide();
        ui->ErosionSpinBox->hide();
        ui->ErosionOkBtn->hide();
    }
}

void MainWindow::on_ErosionSpinBox_valueChanged(int arg1)
{
    ui->ErosionOkBtn->setEnabled(arg1 && arg1 % 2);
}

void MainWindow::on_IncreaseOkBtn_clicked()
{
    StartProcess();
    emit IncreaseStart(myIMG_.get(), ui->ErosionSpinBox->value());
}

void MainWindow::on_IncreaseRadioBtn_toggled(bool checked)
{
    if(checked)
    {
        ui->IncreaseLabel->show();
        ui->IncreaseSpinBox->show();
        ui->IncreaseOkBtn->show();
    }
    else{
        ui->IncreaseLabel->hide();
        ui->IncreaseSpinBox->hide();
        ui->IncreaseOkBtn->hide();
    }
}

void MainWindow::on_IncreaseSpinBox_valueChanged(int arg1)
{
    ui->IncreaseOkBtn->setEnabled(arg1 && (arg1 % 2));
}

void MainWindow::on_HistogramBtn_clicked()
{
    HistArray arr_valsRed = { 0 };
    HistArray arr_valsGreen = { 0 };
    HistArray arr_valsBlue = { 0 };


    auto Cfirst = ConstMyColorIterator::CBegin(*myIMG_);
    auto Clast = ConstMyColorIterator::CEnd(*myIMG_);

    for(; Cfirst != Clast; ++Cfirst)
    {
        arr_valsRed[Cfirst.red()]++;
        arr_valsGreen[Cfirst.green()]++;
        arr_valsBlue[Cfirst.blue()]++;
    }

    Histogram* hist = new Histogram(arr_valsRed, arr_valsGreen, arr_valsBlue, this);
    QObject::connect(hist, &Histogram::finished, &Histogram::deleteLater);
    hist->show();
}

void MainWindow::on_RotateLeftBtn_clicked()
{
    StartProcess();
    emit RotateLeftStart(myIMG_.get());
}

void MainWindow::on_RotateRightBtn_clicked()
{
    StartProcess();
    emit RotateRightStart(myIMG_.get());
}

void MainWindow::on_HMirroredBtn_clicked()
{
    StartProcess();
    emit HMirrorStart(myIMG_.get());
}

void MainWindow::on_VMirroredBtn_clicked()
{
    StartProcess();
    emit VMirrorStart(myIMG_.get());
}

bool isImageFormat(const QString& str)
{
    return str.endsWith(".jpg") || str.endsWith(".jpeg") || str.endsWith(".bmp") || str.endsWith(".png");
}

void MainWindow::on_PrevBtn_clicked()
{
    EnableAll(false);

    if(currFileList_->empty())
        return;

    auto begin = currFileList_->begin();
    auto end = currFileList_->end();

    if(currFileIt_ == begin)
        currFileIt_ = end;

    --currFileIt_;

    while(!isImageFormat(*currFileIt_))
    {
        if(currFileIt_ == begin)
            return;

        --currFileIt_;
    }

    loadImage(*currFileIt_);

    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::on_NextBtn_clicked()
{
    EnableAll(false);

    if(currFileList_->empty())
        return;

    auto begin = currFileList_->begin();
    auto end = currFileList_->end();

    if(currFileIt_ == end)
        currFileIt_ = begin;
    else{
        ++currFileIt_;
        if(currFileIt_ == end)
            currFileIt_ = begin;
    }

    while(!isImageFormat(*currFileIt_))
    {
        ++currFileIt_;

        if(currFileIt_ == end)
            return;
    }

    loadImage(*currFileIt_);

    EnableAll(true);
    ui->ProgressLabel->setText("Okai");
}

void MainWindow::on_QuickSaveBtn_clicked()
{
    if(myIMG_->isNull() || currFileIt_ == currFileList_->end())
        return;

    myIMG_->save(*currFileIt_);
    *tmpIMG_ = *myIMG_;
    ui->CancelBtn->setDisabled(true);
    ui->ProgressLabel->setText("Сохранено");
}
