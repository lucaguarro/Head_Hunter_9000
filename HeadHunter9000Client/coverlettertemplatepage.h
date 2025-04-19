#ifndef COVERLETTERTEMPLATEPAGE_H
#define COVERLETTERTEMPLATEPAGE_H

#include "templatepage.h"

class CoverLetterTemplatePage : public TemplatePage
{
    Q_OBJECT

public:
    explicit CoverLetterTemplatePage(QSettings *settings, QWidget *parent = nullptr);
};

#endif // COVERLETTERTEMPLATEPAGE_H
