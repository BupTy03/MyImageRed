#include "inputmatrix.h"
#include <iostream>

class MyLineEdit : public QLineEdit
{
public:
    MyLineEdit(QWidget* parent = nullptr) : QLineEdit(parent)
    {
        connect(this, &MyLineEdit::editingFinished,
                [this]()
                {
                    if(text() == "")
                    {
                        setText("0.0");
                    }
                });
    }

protected:
    void mousePressEvent(QMouseEvent *e)
    {
        this->clear();
        QLineEdit::mousePressEvent(e);
    }

};

InputMatrix::InputMatrix(QWidget* pwgt, int kernel_size)
    : QDialog(pwgt, Qt::WindowSystemMenuHint)
{
    this->setLayout(new QVBoxLayout(this));
    this->setWindowTitle("Ввод матрицы свёртки");

    vec_input.reserve(kernel_size * kernel_size);
    for(int i = 0; i < kernel_size; i++)
    {
        QHBoxLayout* curr_layout = new QHBoxLayout();

        for(int j = 0; j < kernel_size; j++)
        {
            //QLineEdit* curr_line = new QLineEdit(this);
            MyLineEdit* curr_line = new MyLineEdit(this);

            vec_input.push_back(curr_line);

            curr_line->setText("0.0");

            curr_line->setFixedWidth(50);

            curr_layout->addWidget(curr_line);

            if(j != kernel_size - 1){
                curr_layout->addItem(new QSpacerItem(10, 10));
            }
        }

        this->layout()->addItem(curr_layout);
    }

    QDialogButtonBox* DialBtns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    this->layout()->addWidget(DialBtns);

    connect(DialBtns, SIGNAL(rejected()), this, SLOT(close()));
    connect(DialBtns, SIGNAL(accepted()), this, SLOT(checkInputMatrixValues()));
}

void InputMatrix::checkInputMatrixValues()
{
    for(const auto& i : vec_input)
    {
        QString curr_str = i->text();
        std::replace(curr_str.begin(), curr_str.end(), ',', '.');
        i->setText(curr_str);

        bool isOk = false;
        curr_str.toDouble(&isOk);

        if(!isOk){
            i->setText("0.0");
            QMessageBox::information(this, "Ошибка", "Неверный ввод!", QMessageBox::Ok);
            return;
        }
    }
    this->close();
    emit valuesChecked();
}

vector<double>* InputMatrix::getValuesPtr()
{   
    vector<double>* result = new vector<double>(vec_input.size());

    auto begin_out = vec_input.cbegin();
    auto end_out = vec_input.cend();
    auto begin_in = result->begin();

    for(; begin_out != end_out; ++begin_out, ++begin_in)
        *begin_in = ((*begin_out)->text()).toDouble();

    values.reset(result);

    return values.data();
}


