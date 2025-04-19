#ifndef RESUMETEMPLATEPAGE_H
#define RESUMETEMPLATEPAGE_H

#include "templatepage.h"

class ResumeTemplatePage : public TemplatePage
{
    Q_OBJECT

public:
    explicit ResumeTemplatePage(QSettings *settings, QWidget *parent = nullptr);
};

#endif // RESUMETEMPLATEPAGE_H
