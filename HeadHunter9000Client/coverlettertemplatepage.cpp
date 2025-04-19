#include "coverlettertemplatepage.h"

CoverLetterTemplatePage::CoverLetterTemplatePage(QSettings *settings, QWidget *parent)
    : TemplatePage("Cover Letter",
                   "Write a cover letter for the position of [Job Title] at [Company] "
                   "with the following job description: [Job Description]...",
                   "Open Cover Letter Tester",
                   settings,
                   parent) {}
