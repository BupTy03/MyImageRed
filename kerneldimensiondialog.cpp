#include "kerneldimensiondialog.hpp"

#include <QVBoxLayout>
#include <QSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

KernelDimensionDialog::KernelDimensionDialog(QWidget* parent)
    : QDialog(parent)
    , pSpinBox_{new QSpinBox(this)}
{
    setWindowTitle(QObject::tr("Размер ядра"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto pThisLayout = new QVBoxLayout(this);
    setLayout(pThisLayout);

    constexpr auto step = 3;
    pSpinBox_->setSingleStep(step);
    pSpinBox_->setRange(step, step * 9);
    pThisLayout->addWidget(pSpinBox_);
    QObject::connect(pSpinBox_, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
    [this](auto value) {
        if(value % pSpinBox_->singleStep() != 0)
            pSpinBox_->setValue(pSpinBox_->minimum());
    });

    auto pDialBtns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    pThisLayout->addWidget(pDialBtns);

    QObject::connect(pDialBtns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(pDialBtns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
