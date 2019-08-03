#include "gammaargsdialog.hpp"

GammaArgsDialog::GammaArgsDialog(QWidget* parent)
    : QDialog(parent)
    , coefficientASpinBox_{new QDoubleSpinBox(this)}
    , coefficientGammaSpinBox_{new QDoubleSpinBox(this)}
{
    setWindowTitle(QObject::tr("Коэффициенты гамма-коррекции"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto pThisLayout = new QVBoxLayout(this);
    setLayout(pThisLayout);

    coefficientASpinBox_->setRange(MIN_A_COEFFICIENT, MAX_A_COEFFICIENT);
    pThisLayout->addWidget(coefficientASpinBox_);

    coefficientGammaSpinBox_->setRange(MIN_GAMMA_COEFFICIENT, MAX_GAMMA_COEFFICIENT);
    pThisLayout->addWidget(coefficientGammaSpinBox_);

    auto pDialBtns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    pThisLayout->addWidget(pDialBtns);

    QObject::connect(pDialBtns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    QObject::connect(pDialBtns, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
