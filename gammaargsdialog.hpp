#ifndef GAMMAARGSDIALOG_HPP
#define GAMMAARGSDIALOG_HPP

#include "gammacoefficients.hpp"

#include <QtWidgets>

class GammaArgsDialog : public QDialog
{
    Q_OBJECT
public:
    GammaArgsDialog(QWidget* parent = nullptr);

    inline GammaCoefficients GetGammaCoefficients() const noexcept
    { return {coefficientASpinBox_->value(), coefficientGammaSpinBox_->value()}; }

    static constexpr double MIN_A_COEFFICIENT = 0.01;
    static constexpr double MAX_A_COEFFICIENT = 10.0;

    static constexpr double MIN_GAMMA_COEFFICIENT = 1.0;
    static constexpr double MAX_GAMMA_COEFFICIENT = 10.0;

private:
    QDoubleSpinBox* coefficientASpinBox_ = nullptr;
    QDoubleSpinBox* coefficientGammaSpinBox_ = nullptr;
};

#endif // GAMMAARGSDIALOG_HPP
