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
    , inputMatrixDialog_{new InputMatrixDialog(this)}
    , imageProcessor_{std::make_unique<ImageProcessor>()}
    , myThread_{new QThread}
{
    ui->setupUi(this);
    setWindowTitle(QObject::tr("Обработка изображений"));

    QObject::connect(imageProcessor_.get(), &ImageProcessor::isDone, this, &MainWindow::ProcIsDone);
    QObject::connect(this, &MainWindow::RotateLeftStart,        imageProcessor_.get(), &ImageProcessor::RotateLeftGo);
    QObject::connect(this, &MainWindow::RotateRightStart,       imageProcessor_.get(), &ImageProcessor::RotateRightGo);
    QObject::connect(this, &MainWindow::HMirrorStart,           imageProcessor_.get(), &ImageProcessor::HMirrorGo);
    QObject::connect(this, &MainWindow::VMirrorStart,           imageProcessor_.get(), &ImageProcessor::VMirrorGo);
    QObject::connect(this, &MainWindow::LinearCorrectionStart,  imageProcessor_.get(), &ImageProcessor::LinearCorrGo);
    QObject::connect(this, &MainWindow::GrayWorldStart,         imageProcessor_.get(), &ImageProcessor::GrayWorldGo);
    QObject::connect(this, &MainWindow::GammaStart,             imageProcessor_.get(), &ImageProcessor::GammaFuncGo);
    QObject::connect(this, &MainWindow::GaussBlurStart,         imageProcessor_.get(), &ImageProcessor::GaussBlurGo);
    QObject::connect(this, &MainWindow::MedianStart,            imageProcessor_.get(), &ImageProcessor::MedianFilterGo);
    QObject::connect(this, &MainWindow::CustomStart,            imageProcessor_.get(), &ImageProcessor::CustomFilterGo);
    QObject::connect(this, &MainWindow::ErosionStart,           imageProcessor_.get(), &ImageProcessor::ErosionGo);
    QObject::connect(this, &MainWindow::IncreaseStart,          imageProcessor_.get(), &ImageProcessor::IncreaseGo);

    imageProcessor_->moveToThread(myThread_.get());
    myThread_->start();

    QObject::connect(ui->actionExit,        &QAction::triggered, this, &MainWindow::close);
    QObject::connect(ui->actionSave,        &QAction::triggered, this, &MainWindow::OnActionQuickSave);
    QObject::connect(ui->actionSaveAs,      &QAction::triggered, this, &MainWindow::OnActionSave);
    QObject::connect(ui->actionCancel,      &QAction::triggered, this, &MainWindow::OnActionCancel);
    QObject::connect(ui->actionOpenFile,    &QAction::triggered, this, &MainWindow::OnActionLoad);

    QObject::connect(ui->PrevBtn, &QPushButton::clicked, this, &MainWindow::OnButtonPrev);
    QObject::connect(ui->NextBtn, &QPushButton::clicked, this, &MainWindow::OnButtonNext);

    QObject::connect(ui->actionGaussBlur,               &QAction::triggered, this, &MainWindow::OnActionGaussBlur);
    QObject::connect(ui->actionGrayWorld,               &QAction::triggered, this, &MainWindow::OnActionGrayWorld);
    QObject::connect(ui->actionLinearCorrection,        &QAction::triggered, this, &MainWindow::OnActionLinearCorrection);
    QObject::connect(ui->actionSetConvolutionMatrix,    &QAction::triggered, this, &MainWindow::OnActionCustom);
    QObject::connect(ui->actionHistogram,               &QAction::triggered, this, &MainWindow::OnActionHistogram);

    QObject::connect(inputMatrixDialog_, &InputMatrixDialog::ValuesChecked, this, &MainWindow::OnCustomMatrix);
}

MainWindow::~MainWindow() { delete ui; }


void MainWindow::UpdatePixmap()
{
    if(myIMG_.isNull())
        return;

    QPixmap pm = QPixmap::fromImage(myIMG_);
    ui->label->setPixmap(pm.scaled(ui->label->width(), ui->label->height(), Qt::KeepAspectRatio));
}

bool MainWindow::LoadImage(const QString &str)
{
    if(!myIMG_.load(str))
        return false;

    if(QImage::Format::Format_RGB32 != myIMG_.format()) {
        myIMG_ = myIMG_.convertToFormat(QImage::Format_RGB32);
    }
    tmpIMG_ = myIMG_;
    UpdatePixmap();

    return true;
}

void MainWindow::EnableAll(bool flag)
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

void MainWindow::OnActionSave()
{
    const QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Сохранить как"), QDir::currentPath(), QObject::tr("*.jpg *.jpeg *.png *.bmp"));
    if(fileName.isEmpty())
        return;

    myIMG_.save(fileName);
    ui->ProgressLabel->setText(QObject::tr("Сохранено"));
}

static bool isImageFormat(const QString& str)
{
    return str.endsWith(".jpg") || str.endsWith(".jpeg") || str.endsWith(".bmp") || str.endsWith(".png");
}

void MainWindow::OnActionLoad()
{
    const QString fileName = QFileDialog::getOpenFileName(this, QObject::tr("Открыть файл"), "/", "*.jpg *.jpeg *.png *.bmp");
    if(fileName.isEmpty()) {
        return;
    }

    fileIterator_.LoadFile(fileName);
    fileIterator_.FilterFiles(isImageFormat);

    if(!fileIterator_.HasFiles()) {
        return;
    }

    LoadImage(fileIterator_.Next());

//    LoadImage(fileName);

//    QDir currDir(fileName);
//    currDir.cdUp();

//    currFileList_ = std::make_unique<QStringList>(currDir.entryList(QDir::Files));
//    std::transform(currFileList_->cbegin(), currFileList_->cend(), currFileList_->begin(),
//    [&currDir](const auto& item){
//        return currDir.path() + '/' + item;
//    });

//    currFileIt_ = std::find(currFileList_->begin(), currFileList_->end(), fileName);

    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::OnActionCancel()
{
    myIMG_ = tmpIMG_;
    UpdatePixmap();
    ui->ProgressLabel->setText("");
}

void MainWindow::OnActionLinearCorrection()
{
    StartProcess();
    emit LinearCorrectionStart(&myIMG_);
}

void MainWindow::OnActionGrayWorld()
{
    StartProcess();
    emit GrayWorldStart(&myIMG_);
}


void MainWindow::OnActionGaussBlur()
{
    StartProcess();
    emit GaussBlurStart(&myIMG_);
}

void MainWindow::ProcIsDone()
{
    EnableAll(true);
    UpdatePixmap();
    ui->ProgressLabel->setText(QObject::tr("Готово"));
}

void MainWindow::OnCustomMatrix()
{
    StartProcess();
    emit CustomStart(&myIMG_, &(inputMatrixDialog_->GetValues()));
}

void MainWindow::OnActionCustom() { inputMatrixDialog_->show(); }

void MainWindow::OnActionHistogram()
{
    HistArray arr_valsRed   = { 0 };
    HistArray arr_valsGreen = { 0 };
    HistArray arr_valsBlue  = { 0 };

    auto Cfirst = ConstMyColorIterator::CBegin(myIMG_);
    auto Clast = ConstMyColorIterator::CEnd(myIMG_);

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

void MainWindow::OnActionRotateLeft()
{
    StartProcess();
    emit RotateLeftStart(&myIMG_);
}

void MainWindow::OnActionRotateRight()
{
    StartProcess();
    emit RotateRightStart(&myIMG_);
}

void MainWindow::OnActionHMirrored()
{
    StartProcess();
    emit HMirrorStart(&myIMG_);
}

void MainWindow::OnActionVMirrored()
{
    StartProcess();
    emit VMirrorStart(&myIMG_);
}

void MainWindow::OnButtonPrev()
{
    EnableAll(false);

    if(!fileIterator_.HasFiles()) {
        return;
    }

    LoadImage(fileIterator_.Prev());

//    if(currFileList_->empty())
//        return;

//    auto begin = currFileList_->begin();
//    auto end = currFileList_->end();

//    if(currFileIt_ == begin)
//        currFileIt_ = end;

//    --currFileIt_;

//    while(!isImageFormat(*currFileIt_))
//    {
//        if(currFileIt_ == begin)
//            return;

//        --currFileIt_;
//    }

//    LoadImage(*currFileIt_);



    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::OnButtonNext()
{
    EnableAll(false);

    if(!fileIterator_.HasFiles()) {
        return;
    }

    LoadImage(fileIterator_.Next());

//    if(currFileList_->empty())
//        return;

//    auto begin = currFileList_->begin();
//    auto end = currFileList_->end();

//    if(currFileIt_ == end)
//        currFileIt_ = begin;
//    else{
//        ++currFileIt_;
//        if(currFileIt_ == end)
//            currFileIt_ = begin;
//    }

//    while(!isImageFormat(*currFileIt_))
//    {
//        ++currFileIt_;

//        if(currFileIt_ == end)
//            return;
//    }

//    LoadImage(*currFileIt_);

    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::OnActionQuickSave()
{
    if(myIMG_.isNull() || currFileIt_ == currFileList_->end())
        return;

    myIMG_.save(*currFileIt_);
    tmpIMG_ = myIMG_;
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
