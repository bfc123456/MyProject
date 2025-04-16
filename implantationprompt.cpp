#include "implantationprompt.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ImplantationPrompt::ImplantationPrompt(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("提示");
    setFixedSize(400, 200);

    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* label = new QLabel("请将传感器植入患者体内，点击“下一步”");
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 16px;");
    layout->addWidget(label);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* backButton = new QPushButton("返回");
    QPushButton* nextButton = new QPushButton("下一步");
    backButton->setStyleSheet("background-color: #0078d7; color: white;");
    nextButton->setStyleSheet("background-color: #4CAF50; color: white;");

    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);
    buttonLayout->addSpacing(40);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    connect(backButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(nextButton, &QPushButton::clicked, this, &QDialog::accept);
}
ImplantationPrompt::~ImplantationPrompt(){

}
