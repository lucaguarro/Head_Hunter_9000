#ifndef JOB_H
#define JOB_H

#include <QString>

struct Job
{
    int id;
    QString companyName;
    QString location;
    QString jobTitle;
    QString description;
    QString createdAt; // ISO date string
    int preferenceScore;
};

#endif // JOB_H
