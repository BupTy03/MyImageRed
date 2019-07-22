#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "myimageproc.h"
#include "imageprocessor.hpp"
#include "inputmatrixdialog.hpp"

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

    QObject::connect(imgProc_.get(), &ImageProcessor::isDone, this, &MainWindow::ProcIsDone);
    QObject::connect(this, &MainWindow::RotateLeftStart, imgProc_.get(), &ImageProcessor::RotateLeftGo);
    QObject::connect(this, &MainWindow::RotateRightStart, imgProc_.get(), &ImageProcessor::RotateRightGo);
    QObject::connect(this, &MainWindow::HMirrorStart, imgProc_.get(), &ImageProcessor::HMirrorGo);
    QObject::connect(this, &MainWindow::VMirrorStart, imgProc_.get(), &ImageProcessor::VMirrorGo);
    QObject::connect(this, &MainWindow::LinCorrStart, imgProc_.get(), &ImageProcessor::LinearCorrGo);
    QObject::connect(this, &MainWindow::GrayWorldStart, imgProc_.get(), &ImageProcessor::GrayWorldGo);
    QObject::connect(this, &MainWindow::GammaStart, imgProc_.get(), &ImageProcessor::GammaFuncGo);
    QObject::connect(this, &MainWindow::GBStart, imgProc_.get(), &ImageProcessor::GaussBlurGo);
    QObject::connect(this, &MainWindow::MedianStart, imgProc_.get(), &ImageProcessor::MedianFilterGo);
    QObject::connect(this, &MainWindow::CustomStart, imgProc_.get(), &ImageProcessor::CustomFilterGo);
    QObject::connect(this, &MainWindow::ErosionStart, imgProc_.get(), &ImageProcessor::ErosionGo);
    QObject::connect(this, &MainWindow::IncreaseStart, imgProc_.get(), &ImageProcessor::IncreaseGo);

    imgProc_->moveToThread(myThread_.get());
    myThread_->start();


    QObject::connect(this, &MainWindow::RotateLeftStart, imgProc_.get(), &ImageProcessor::RotateLeftGo);
    QObject::connect(this, &MainWindow::RotateRightStart, imgProc_.get(), &ImageProcessor::RotateRightGo);
    QObject::connect(this, &MainWindow::HMirrorStart, imgProc_.get(), &ImageProcessor::HMirrorGo);
    QObject::connect(this, &MainWindow::VMirrorStart, imgProc_.get(), &ImageProcessor::VMirrorGo);

    QObject::connect(ui->actionExit, &QAction::triggered, this, &MainWindow::on_Quit_clicked);
    QObject::connect(ui->actionSave, &QAction::triggered, this, &MainWindow::on_QuickSaveBtn_clicked);
    QObject::connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::on_SaveBtn_clicked);
    QObject::connect(ui->actionCancel, &QAction::triggered, this, &MainWindow::on_CancelBtn_clicked);
    QObject::connect(ui->actionOpenFile, &QAction::triggered, this, &MainWindow::on_LoadBtn_clicked);

    QObject::connect(ui->actionGaussBlur, &QAction::triggered, this, &MainWindow::on_GBOkBtn_clicked);
    QObject::connect(ui->actionGrayWorld, &QAction::triggered, this, &MainWindow::on_GrayWorldBtn_clicked);
    QObject::connect(ui->actionLinearCorrection, &QAction::triggered, this, &MainWindow::on_LinCorrBtn_clicked);
    QObject::connect(ui->actionSetConvolutionMatrix, &QAction::triggered, this, &MainWindow::on_CustomBtn_clicked);
    QObject::connect(ui->actionHistogram, &QAction::triggered, this, &MainWindow::on_HistogramBtn_clicked);

    QObject::connect(inMtx_, &InputMatrixDialog::valuesChecked, this, &MainWindow::CustomMatrix);
}

MainWindow::~MainWindow() { delete ui; }


void MainWindow::updatePixmap()
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
    updatePixmap();

    return true;
}

void MainWindow::EnableAll(const bool flag)
{
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
    ui->ProgressLabel->setText(QObject::tr("Обработка..."));
    EnableAll(false);
}

void MainWindow::on_SaveBtn_clicked()
{
    const QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Сохранить как"), QDir::currentPath(), QObject::tr("*.jpg *.jpeg *.png *.bmp"));
    if(fileName.isEmpty())
        return;

    myIMG_->save(fileName);
    ui->ProgressLabel->setText(QObject::tr("Сохранено"));
}

void MainWindow::on_LoadBtn_clicked()
{
    const QString fileName = QFileDialog::getOpenFileName(this, QObject::tr("Открыть файл"), "/", "*.jpg *.jpeg *.png *.bmp");
    loadImage(fileName);

    QDir currDir(fileName);
    currDir.cdUp();

    currFileList_ = std::make_unique<QStringList>(currDir.entryList(QDir::Files));
    std::transform(currFileList_->cbegin(), currFileList_->cend(), currFileList_->begin(),
    [&currDir](const auto& item){
        return currDir.path() + '/' + item;
    });

    currFileIt_ = std::find(currFileList_->begin(), currFileList_->end(), fileName);

    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::on_CancelBtn_clicked()
{
    *myIMG_ = *tmpIMG_;
    updatePixmap();
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


void MainWindow::on_GBOkBtn_clicked()
{
    StartProcess();
    emit GBStart(myIMG_.get());
}

void MainWindow::ProcIsDone()
{
    EnableAll(true);
    updatePixmap();
    ui->ProgressLabel->setText(QObject::tr("Готово"));
}

void MainWindow::CustomMatrix()
{
    StartProcess();
    emit CustomStart(myIMG_.get(), &(inMtx_->getValues()));
}

void MainWindow::on_CustomBtn_clicked() { inMtx_->show(); }

void MainWindow::on_Quit_clicked()
{
    close();
}



void MainWindow::on_HistogramBtn_clicked()
{
    HistArray arr_valsRed   = { 0 };
    HistArray arr_valsGreen = { 0 };
    HistArray arr_valsBlue  = { 0 };

    auto Cfirst = ConstMyColorIterator::CBegin(*myIMG_);
    auto Clast = ConstMyColorIterator::CEnd(*myIMG_);

    for(; Cfirst != Clast; ++Cfirst)
    {
        arr_valsRed[Cfirst.red()]++;
        arr_valsGreen[Cfirst.green()]++;
        arr_valsBlue[Cfirst.blue()]++;
    }

    auto hist = new Histogram(arr_valsRed, arr_valsGreen, arr_valsBlue, this);
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
    //ui->ProgressLabel->setText("Okai");
}

void MainWindow::on_QuickSaveBtn_clicked()
{
    if(myIMG_->isNull() || currFileIt_ == currFileList_->end())
        return;

    myIMG_->save(*currFileIt_);
    *tmpIMG_ = *myIMG_;
    ui->ProgressLabel->setText(QObject::tr("Сохранено"));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    const auto reply = QMessageBox::question(this, QObject::tr("Выход"), QObject::tr("Вы хотите завершить приложение ?"),
                                       QMessageBox::Yes | QMessageBox::No);

    if(reply == QMessageBox::Yes)
        event->accept();
    else
        event->ignore();
}
