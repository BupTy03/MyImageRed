#include "inputmatrixdialog.hpp"

class MyLineEdit : public QLineEdit
{
public:
    explicit MyLineEdit(QWidget* parent = nullptr) : QLineEdit(parent)
    {
        QObject::connect(this, &MyLineEdit::editingFinished, this, &MyLineEdit::onEditingFinished);
    }

private slots:
    void onEditingFinished() { if((text()).isEmpty()) setText("0.0"); }

protected:
    void mousePressEvent(QMouseEvent *e) override;
};

void MyLineEdit::mousePressEvent(QMouseEvent *e)
{
    clear();
    QLineEdit::mousePressEvent(e);
}



InputMatrixDialog::InputMatrixDialog(QWidget* parent, int kernel_size)
    : QDialog{parent, Qt::WindowSystemMenuHint}
{
    setLayout(new QVBoxLayout(this));
    setWindowTitle(QObject::tr("Ввод матрицы свёртки"));

    inputLines_.reserve(static_cast<std::size_t>(kernel_size * kernel_size));
    for(int i = 0; i < kernel_size; i++)
    {
        auto curr_layout = new QHBoxLayout();

        for(int j = 0; j < kernel_size; j++)
        {
            auto curr_line = new MyLineEdit(this);
            inputLines_.push_back(curr_line);

            curr_line->setText("0.0");
            curr_line->setFixedWidth(50);
            curr_layout->addWidget(curr_line);

            if(j != kernel_size - 1){
                curr_layout->addItem(new QSpacerItem(10, 10));
            }
        }

        layout()->addItem(curr_layout);
    }

    QDialogButtonBox* dialBtns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    layout()->addWidget(dialBtns);

    QObject::connect(dialBtns, &QDialogButtonBox::rejected, this, &QDialogButtonBox::close);
    QObject::connect(dialBtns, &QDialogButtonBox::accepted, this, &InputMatrixDialog::checkInputMatrixValues);
}

void InputMatrixDialog::checkInputMatrixValues()
{
    for(const auto line : inputLines_)
    {
        QString currStr = line->text();
        currStr.replace(',', '.');
        line->setText(currStr);

        bool isOk = false;
        currStr.toDouble(&isOk);

        if(!isOk){
            line->setText("0.0");
            QMessageBox::information(this, QObject::tr("Ошибка"), QObject::tr("Неверный ввод!"), QMessageBox::Ok);
            return;
        }
    }
    this->close();
    emit valuesChecked();
}

const std::vector<double>& InputMatrixDialog::getValues() const
{   
    values_.clear();
    values_.reserve(inputLines_.size());
    std::transform(std::cbegin(inputLines_), std::cend(inputLines_), std::back_inserter(values_),
    [](const auto line){
        return (line->text()).toDouble();
    });

    return values_;
}


