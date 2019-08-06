#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "myimageproc.h"
#include "imageprocessor.hpp"
#include "inputmatrixdialog.hpp"
#include "kerneldimensiondialog.hpp"
#include "gammaargsdialog.hpp"

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
    , imgHolder_{std::make_unique<ImageHolder>()}
    , imageProcessor_{std::make_unique<ImageProcessor>()}
    , myThread_{new QThread}
{
    ui->setupUi(this);
    setWindowTitle(QObject::tr("Обработка изображений"));

    QObject::connect(ui->actionExit,        &QAction::triggered, this, &MainWindow::close);
    QObject::connect(ui->actionSave,        &QAction::triggered, this, &MainWindow::OnActionQuickSave);
    QObject::connect(ui->actionSaveAs,      &QAction::triggered, this, &MainWindow::OnActionSave);
    QObject::connect(ui->actionCancel,      &QAction::triggered, [this]{ myIMG_ = imgHolder_->RevertImage(); UpdatePixmap(); });
    QObject::connect(ui->actionOpenFile,    &QAction::triggered, this, &MainWindow::OnActionLoad);

    QObject::connect(ui->PrevBtn,           &QPushButton::clicked, imgHolder_.get(), &ImageHolder::LoadPreviousImageFile);
    QObject::connect(ui->NextBtn,           &QPushButton::clicked, imgHolder_.get(), &ImageHolder::LoadNextImageFile);

    QObject::connect(ui->PrevStateBtn, &QPushButton::clicked, [this] {
        auto pImage = imgHolder_->PrevImageState();
        if(pImage) {
            myIMG_ = std::move(pImage);
            UpdatePixmap();
        }
    });

    QObject::connect(ui->NextStateBtn, &QPushButton::clicked, [this] {
        auto pImage = imgHolder_->NextImageState();
        if(pImage) {
            myIMG_ = std::move(pImage);
            UpdatePixmap();
        }
    });

    QObject::connect(ui->RotateLeftBtn,     &QPushButton::clicked, [this]{ StartProcess();  emit ProcessImage(RotateLeft); });
    QObject::connect(ui->RotateRightBtn,    &QPushButton::clicked, [this]{ StartProcess();  emit ProcessImage(RotateRight); });
    QObject::connect(ui->HMirroredBtn,      &QPushButton::clicked, [this]{ StartProcess();  emit ProcessImage([](QImage& img){ img = img.mirrored(false, true); }); });
    QObject::connect(ui->VMirroredBtn,      &QPushButton::clicked, [this]{ StartProcess();  emit ProcessImage([](QImage& img){ img = img.mirrored(true, false); }); });

    QObject::connect(ui->actionGaussBlur,               &QAction::triggered, [this]{ StartProcess();  emit ProcessImage(GaussBlur); });
    QObject::connect(ui->actionGrayWorld,               &QAction::triggered, [this]{ StartProcess();  emit ProcessImage(GrayWorld); });
    QObject::connect(ui->actionLinearCorrection,        &QAction::triggered, [this]{ StartProcess();  emit ProcessImage(LinearCorrection); });
    QObject::connect(ui->actionSetConvolutionMatrix,    &QAction::triggered, [this]{ inputMatrixDialog_->show(); });

    QObject::connect(ui->actionMedianFilter, &QAction::triggered, [this] {
        auto dial = std::make_unique<KernelDimensionDialog>(this);
        if(dial->exec() != QDialog::DialogCode::Accepted)
            return;

        const auto value = dial->GetValue();
        StartProcess();
        emit ProcessImage([value](QImage& img) { MedianFilter(img, value); });
    });

    QObject::connect(ui->actionErosion, &QAction::triggered, [this] {
        auto dial = std::make_unique<KernelDimensionDialog>(this);
        if(dial->exec() != QDialog::DialogCode::Accepted)
            return;

        const auto value = dial->GetValue();
        StartProcess();
        emit ProcessImage([value](QImage& img) { Erosion(img, value); });
    });

    QObject::connect(ui->actionIncrease, &QAction::triggered, [this] {
        auto dial = std::make_unique<KernelDimensionDialog>(this);
        if(dial->exec() != QDialog::DialogCode::Accepted)
            return;

        const auto value = dial->GetValue();
        StartProcess();
        emit ProcessImage([value](QImage& img) { Increase(img, value); });
    });

    QObject::connect(ui->actionGammaCorrection, &QAction::triggered, [this] {
        auto dial = std::make_unique<GammaArgsDialog>();
        if(dial->exec() != QDialog::DialogCode::Accepted)
            return;

        StartProcess();
        emit ProcessImage([coeff = dial->GetGammaCoefficients()](QImage& img){ GammaFunc(img, coeff.A, coeff.Gamma); });
    });

    QObject::connect(this, &MainWindow::GetHistogram, imgHolder_.get(), &ImageHolder::StartHistogram);
    QObject::connect(imgHolder_.get(), &ImageHolder::HistogramDone, this, &MainWindow::ShowHistogram);
    QObject::connect(ui->actionHistogram, &QAction::triggered, [this]{ StartProcess(); emit GetHistogram(); });

    QObject::connect(ui->QuickSaveBtn, &QPushButton::clicked, this, &MainWindow::OnActionQuickSave);
    QObject::connect(imgHolder_.get(), &ImageHolder::ProcessingDone, this, &MainWindow::ProcIsDone);
    QObject::connect(imgHolder_.get(), &ImageHolder::ImageLoaded, this, &MainWindow::ProcIsDone);
    QObject::connect(this, &MainWindow::StartLoadingImage, imgHolder_.get(), &ImageHolder::LoadImageFile);
    QObject::connect(this, &MainWindow::ProcessImage, imgHolder_.get(), &ImageHolder::StartImageProcessing);

    QObject::connect(inputMatrixDialog_, &InputMatrixDialog::ValuesChecked,
    [this, &vec = inputMatrixDialog_->GetValues()]
    {
        StartProcess();
        emit ProcessImage([&vec](QImage& img){ CustomFilter(img, vec); });
    });

    imgHolder_->moveToThread(myThread_.get());
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

bool MainWindow::LoadNewImage(const QString &str)
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
    UpdatePixmap();

    return true;
}

void MainWindow::EnableAll(bool flag)
{
    ui->HMirroredBtn->setEnabled(flag);
    ui->NextBtn->setEnabled(flag);
    ui->PrevBtn->setEnabled(flag);
    ui->NextStateBtn->setEnabled(flag);
    ui->PrevStateBtn->setEnabled(flag);
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

void MainWindow::OnActionLoad()
{
    const QString fileName = QFileDialog::getOpenFileName(this, QObject::tr("Открыть файл"), "/", "*.jpg *.jpeg *.png *.bmp");
    EnableAll(false);

    if(fileName.isEmpty()) return;

    emit StartLoadingImage(fileName);
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

void MainWindow::ProcIsDone(std::shared_ptr<const QImage> pImage)
{
    assert(pImage);
    myIMG_ = pImage;
    EnableAll(true);
    UpdatePixmap();
    ui->ProgressLabel->setText(QObject::tr("Готово"));
}

void MainWindow::ShowHistogram(HistRGB hist)
{
    EnableAll(true);
    ui->ProgressLabel->setText(QObject::tr("Готово"));

    auto histogram = new Histogram(hist, this);
    QObject::connect(histogram, &Histogram::finished, &Histogram::deleteLater);
    histogram->show();
}

void MainWindow::OnCustomMatrix()
{
    StartProcess();
    emit CustomStart(*myIMG_, inputMatrixDialog_->GetValues());
}

void MainWindow::OnActionCustom() {  }

void MainWindow::OnActionHistogram()
{
//    HistArray arr_valsRed   = { 0 };
//    HistArray arr_valsGreen = { 0 };
//    HistArray arr_valsBlue  = { 0 };

//    auto Cfirst = ConstMyColorIterator::CBegin(*myIMG_);
//    auto Clast = ConstMyColorIterator::CEnd(*myIMG_);

//    for(; Cfirst != Clast; ++Cfirst)
//    {
//        arr_valsRed[Cfirst.red()]++;
//        arr_valsGreen[Cfirst.green()]++;
//        arr_valsBlue[Cfirst.blue()]++;
//    }

//    Hist valsRed;
//    Hist valsGreen;
//    Hist valsBlue;

//    std::tie(valsRed, valsGreen, valsBlue) = MakeHistogram(*myIMG_);

//    auto hist = new Histogram( this);
//    QObject::connect(hist, &Histogram::finished, &Histogram::deleteLater);
//    hist->show();
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
    LoadNewImage(*fileIterator_);

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
    LoadNewImage(*fileIterator_);

    EnableAll(true);
    ui->ProgressLabel->setText("");
}

void MainWindow::OnActionQuickSave()
{
    assert(myIMG_);
    if(myIMG_->isNull() || !fileIterator_.HasFiles())
        return;

    myIMG_->save(*fileIterator_);
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
