#include "resumetemplatepage.h"

ResumeTemplatePage::ResumeTemplatePage(QWidget *parent)
    : TemplatePage("Resume",
                   "Write a resume for the position of [Job Title] at [Company] "
                   "with the following job description: [Job Description]...",
                   "Open Template Tester",
                   parent) {}
