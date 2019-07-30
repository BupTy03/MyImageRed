#ifndef KERNELDIMENSIONDIALOG_HPP
#define KERNELDIMENSIONDIALOG_HPP

#include <QDialog>
#include <QSpinBox>

class KernelDimensionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KernelDimensionDialog(QWidget* parent = nullptr);
    inline int GetValue() const { return pSpinBox_->value(); }

private:
    QSpinBox* pSpinBox_ = nullptr;
};

#endif // KERNELDIMENSIONDIALOG_HPP
