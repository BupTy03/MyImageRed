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
#include <QMetaType>

using namespace QtCharts;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui{new Ui::MainWindow}
    , inputMatrixDialog_{new InputMatrixDialog(this)}
    , imageProcessor_{std::make_unique<ImageProcessor>()}
    , myThread_{new QThread}
{
    ui->setupUi(this);
    setWindowTitle(QObject::tr("Обработка изображений"));

    qRegisterMetaType<std::shared_ptr<QImage>>("std::shared_ptr<QImage>");

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

    QObject::connect(ui->actionExit,        &QAction::triggered, this, &MainWindow::close);
    QObject::connect(ui->actionSave,        &QAction::triggered, this, &MainWindow::OnActionQuickSave);
    QObject::connect(ui->actionSaveAs,      &QAction::triggered, this, &MainWindow::OnActionSave);
    QObject::connect(ui->actionCancel,      &QAction::triggered, this, &MainWindow::OnActionCancel);
    QObject::connect(ui->actionOpenFile,    &QAction::triggered, this, &MainWindow::OnActionLoad);

    QObject::connect(ui->PrevBtn,           &QPushButton::clicked, this, &MainWindow::OnButtonPrev);
    QObject::connect(ui->NextBtn,           &QPushButton::clicked, this, &MainWindow::OnButtonNext);
    QObject::connect(ui->RotateLeftBtn,     &QPushButton::clicked, this, &MainWindow::OnButtonRotateLeft);
    QObject::connect(ui->RotateRightBtn,    &QPushButton::clicked, this, &MainWindow::OnButtonRotateRight);
    QObject::connect(ui->HMirroredBtn,      &QPushButton::clicked, this, &MainWindow::OnButtonHMirrored);
    QObject::connect(ui->VMirroredBtn,      &QPushButton::clicked, this, &MainWindow::OnButtonVMirrored);
    QObject::connect(ui->QuickSaveBtn,      &QPushButton::clicked, this, &MainWindow::OnActionQuickSave);

    QObject::connect(ui->actionGaussBlur,               &QAction::triggered, this, &MainWindow::OnActionGaussBlur);
    QObject::connect(ui->actionGrayWorld,               &QAction::triggered, this, &MainWindow::OnActionGrayWorld);
    QObject::connect(ui->actionLinearCorrection,        &QAction::triggered, this, &MainWindow::OnActionLinearCorrection);
    QObject::connect(ui->actionSetConvolutionMatrix,    &QAction::triggered, this, &MainWindow::OnActionCustom);
    QObject::connect(ui->actionHistogram,               &QAction::triggered, this, &MainWindow::OnActionHistogram);

    QObject::connect(inputMatrixDialog_, &InputMatrixDialog::ValuesChecked, this, &MainWindow::OnCustomMatrix);

    imageProcessor_->moveToThread(myThread_.get());
    myThread_->start();
}

MainWindow::~MainWindow() { delete ui; }


void MainWindow::UpdatePixmap()
{
    if(!myIMG_ || myIMG_->isNull()){
        return;
    }

    auto pm = QPixmap::fromImage(*myIMG_);
    ui->label->setPixmap(pm.scaled(ui->label->width(), ui->label->height(), Qt::KeepAspectRatio));
}

bool MainWindow::LoadImage(const QString &str)
{
    auto newImg = std::make_unique<QImage>();
    if(!newImg->load(str)){
        return false;
    }

    if(QImage::Format::Format_RGB32 != newImg->format()) {
        myIMG_ = std::make_shared<QImage>(myIMG_->convertToFormat(QImage::Format_RGB32));
    }
    else {
        myIMG_ = std::move(newImg);
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
    assert(myIMG_);
    const QString fileName = QFileDialog::getSaveFileName(this, QObject::tr("Сохранить как"), QDir::currentPath(), QObject::tr("*.jpg *.jpeg *.png *.bmp"));
    if(fileName.isEmpty())
        return;

    myIMG_->save(fileName);
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

    LoadImage(*fileIterator_);

    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::OnActionCancel()
{
    assert(tmpIMG_);
    myIMG_ = tmpIMG_;
    UpdatePixmap();
    ui->ProgressLabel->setText("");
}

void MainWindow::OnActionLinearCorrection()
{
    StartProcess();
    emit LinearCorrectionStart(*myIMG_);
}

void MainWindow::OnActionGrayWorld()
{
    StartProcess();
    emit GrayWorldStart(*myIMG_);
}


void MainWindow::OnActionGaussBlur()
{
    StartProcess();
    emit GaussBlurStart(*myIMG_);
}

void MainWindow::ProcIsDone(std::shared_ptr<QImage> pImage)
{
    assert(pImage);
    myIMG_ = pImage;
    EnableAll(true);
    UpdatePixmap();
    ui->ProgressLabel->setText(QObject::tr("Готово"));
}

void MainWindow::OnCustomMatrix()
{
    StartProcess();
    emit CustomStart(*myIMG_, inputMatrixDialog_->GetValues());
}

void MainWindow::OnActionCustom() { inputMatrixDialog_->show(); }

void MainWindow::OnActionHistogram()
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

void MainWindow::OnButtonRotateLeft()
{
    StartProcess();
    emit RotateLeftStart(*myIMG_);
}

void MainWindow::OnButtonRotateRight()
{
    StartProcess();
    emit RotateRightStart(*myIMG_);
}

void MainWindow::OnButtonHMirrored()
{
    StartProcess();
    emit HMirrorStart(*myIMG_);
}

void MainWindow::OnButtonVMirrored()
{
    StartProcess();
    emit VMirrorStart(*myIMG_);
}

void MainWindow::OnButtonPrev()
{
    EnableAll(false);

    if(!fileIterator_.HasFiles()) {
        return;
    }

    --fileIterator_;
    LoadImage(*fileIterator_);

    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::OnButtonNext()
{
    EnableAll(false);

    if(!fileIterator_.HasFiles()) {
        return;
    }

    ++fileIterator_;
    LoadImage(*fileIterator_);

    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::OnActionQuickSave()
{
    assert(myIMG_);
    if(myIMG_->isNull() || !fileIterator_.HasFiles())
        return;

    myIMG_->save(*fileIterator_);
    tmpIMG_ = myIMG_;
    ui->ProgressLabel->setText(QObject::tr("Сохранено"));
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    assert(event != nullptr);
    const auto reply = QMessageBox::question(this, QObject::tr("Выход"), QObject::tr("Вы хотите завершить приложение ?"),
                                       QMessageBox::Yes | QMessageBox::No);

    if(reply == QMessageBox::Yes)
        event->accept();
    else
        event->ignore();
}
