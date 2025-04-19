#include "resumetemplatepage.h"

ResumeTemplatePage::ResumeTemplatePage(QSettings *settings, QWidget *parent)
    : TemplatePage("Resume",
                   "Write a resume for the position of [Job Title] at [Company] "
                   "with the following job description: [Job Description]...",
                   "Open Template Tester",
                   settings,
                   parent) {}
