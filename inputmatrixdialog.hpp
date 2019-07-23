#ifndef INPUTMATRIXDIALOG_H
#define INPUTMATRIXDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QDialogButtonBox>

#include <vector>
#include <memory>

class InputMatrixDialog : public QDialog
{
    Q_OBJECT
public:
    InputMatrixDialog(QWidget* parent = nullptr, int kernel_size = 7);
    const std::vector<double>& GetValues() const;

signals:
    void ValuesChecked();

private slots:
    void CheckInputMatrixValues();

private:
    std::vector<QLineEdit*> inputLines_;
    mutable std::vector<double> values_;
};

#endif // INPUTMATRIXDIALOG_H
