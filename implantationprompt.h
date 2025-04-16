#ifndef IMPLANTATIONPROMPT_H
#define IMPLANTATIONPROMPT_H

#include <QDialog>

class ImplantationPrompt : public QDialog
{
    Q_OBJECT
public:
    explicit ImplantationPrompt(QWidget* parent = nullptr);
    ~ImplantationPrompt();
};

#endif // IMPLANTATIONPROMPT_H
