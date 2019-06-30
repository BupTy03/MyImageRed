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

using namespace std;

class InputMatrixDialog : public QDialog
{
        Q_OBJECT

public:
    InputMatrixDialog(QWidget* pwgt = nullptr, int kernel_size = 7);
    std::vector<double>* getValuesPtr();

signals:
    void valuesChecked();

private slots:
    void checkInputMatrixValues();

private:
    vector<QLineEdit*> vec_input;
    QScopedPointer<vector<double>> values;
};

#endif // INPUTMATRIXDIALOG_H
