#ifndef INPUTMATRIX_H
#define INPUTMATRIX_H

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

class InputMatrix : public QDialog
{
        Q_OBJECT

public:
    InputMatrix(QWidget* pwgt = nullptr, int kernel_size = 7);
    vector<double>* getValuesPtr();

signals:
    void valuesChecked();

private slots:
    void checkInputMatrixValues();

private:
    vector<QLineEdit*> vec_input;
    QScopedPointer<vector<double>> values;
};

#endif // INPUTMATRIX_H
