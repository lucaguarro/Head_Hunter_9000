#include "joblistingsui.h"
#include "sidebarjoblistwidget.h"
#include "starratingwidget.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QDebug>

JobListingsUI::JobListingsUI(QWidget *parent, DatabaseManager *dbManager, SidebarJobListWidget *sidebar)
    : QWidget(parent), dbManager(dbManager), sidebar(sidebar), currentIndex(0)
{
    setupUI();

    // Connect Sidebar Selection Signal
    connect(sidebar, &SidebarJobListWidget::jobSelected, this, &JobListingsUI::handleSidebarSelection);

    // Display the first job if available
    if (!sidebar->getJobs().isEmpty()) {
        displayCurrentJob();
    }
}

void JobListingsUI::setupUI()
{
    // Initialize Labels with Word Wrap Enabled
    companyNameLabel = new QLabel(this);
    companyNameLabel->setWordWrap(true); // Enable word wrap

    locationLabel = new QLabel(this);
    locationLabel->setWordWrap(true); // Enable word wrap

    jobTitleLabel = new QLabel(this);
    jobTitleLabel->setWordWrap(true); // Enable word wrap

    descriptionLabel = new QLabel(this);
    descriptionLabel->setWordWrap(true); // Enable word wrap

    createdAtLabel = new QLabel(this);
    createdAtLabel->setWordWrap(true); // Enable word wrap

    // Initialize StarRatingWidget
    starRatingWidget = new StarRatingWidget(this);
    starRatingWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // Ensure proper sizing
    connect(starRatingWidget, &StarRatingWidget::ratingChanged, this, &JobListingsUI::updatePreferenceScore);

    // Initialize Navigation Buttons
    prevButton = new QPushButton("<", this);
    connect(prevButton, &QPushButton::clicked, this, &JobListingsUI::showPreviousJob);

    nextButton = new QPushButton(">", this);
    connect(nextButton, &QPushButton::clicked, this, &JobListingsUI::showNextJob);

    // Create a Container Widget for Scroll Area
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->addWidget(companyNameLabel);
    contentLayout->addWidget(locationLabel);
    contentLayout->addWidget(jobTitleLabel);
    contentLayout->addWidget(descriptionLabel);
    contentLayout->addWidget(createdAtLabel);
    contentLayout->setAlignment(Qt::AlignTop); // Align content to the top

    // Initialize Scroll Area
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); // Allow the scroll area to resize with the window
    scrollArea->setWidget(contentWidget);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Layout Setup
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(scrollArea); // Add Scroll Area to the main layout

    // Create a horizontal layout for the StarRatingWidget
    QHBoxLayout *ratingLayout = new QHBoxLayout();
    ratingLayout->addStretch(); // Add expandable space before the widget
    ratingLayout->addWidget(starRatingWidget); // Add the StarRatingWidget
    ratingLayout->addStretch(); // Add expandable space after the widget

    // Add the rating layout to the main layout
    mainLayout->addLayout(ratingLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(nextButton);

    mainLayout->addLayout(buttonLayout); // Add Navigation Buttons below StarRatingWidget

    setLayout(mainLayout);
}

void JobListingsUI::displayCurrentJob()
{
    QList<Job> &jobList = sidebar->getJobs();

    if (jobList.isEmpty() || currentIndex < 0 || currentIndex >= jobList.size()) {
        companyNameLabel->setText("No job postings available.");
        locationLabel->clear();
        jobTitleLabel->clear();
        descriptionLabel->clear();
        createdAtLabel->clear();
        starRatingWidget->setRating(0, false);
        prevButton->setEnabled(false);
        nextButton->setEnabled(false);
        return;
    }

    Job &job = jobList[currentIndex];


    // Set Company Name
    companyNameLabel->setText("<b>Company:</b> " + job.companyName.toHtmlEscaped());

    // Set Location
    locationLabel->setText("<b>Location:</b> " + job.location.toHtmlEscaped());

    // Set Job Title
    jobTitleLabel->setText("<b>Title:</b> " + job.jobTitle.toHtmlEscaped());

    // Set Description with Proper Formatting
    QString description = "<b>Description:</b> " + job.description.toHtmlEscaped();
    description.replace("\n", "<br>"); // Replace newline characters with <br> tags
    descriptionLabel->setText(description);

    // Set Created At
    createdAtLabel->setText("<b>Posted:</b> " + job.createdAt.toHtmlEscaped());

    // Update StarRatingWidget
    starRatingWidget->setRating(job.preferenceScore, false);

    // Update Navigation Buttons
    prevButton->setEnabled(currentIndex > 0);
    nextButton->setEnabled(currentIndex < jobList.size() - 1);
}

void JobListingsUI::showPreviousJob()
{
    if (currentIndex > 0) {
        --currentIndex;
        displayCurrentJob();
    }
}

void JobListingsUI::showNextJob()
{
    if (currentIndex < sidebar->getJobs().size() - 1) {
        ++currentIndex;
        displayCurrentJob();
    }
}

void JobListingsUI::handleSidebarSelection(int index)
{
    if (index >= 0 && index < sidebar->getJobs().size()) {
        currentIndex = index;
        displayCurrentJob();
    }
}

void JobListingsUI::updatePreferenceScore(int score)
{
    QList<Job> &jobList = sidebar->getJobs();

    if (jobList.isEmpty() || currentIndex < 0 || currentIndex >= jobList.size()) {
        qWarning() << "No job to update the preference score.";
        return;
    }

    // Update the preference score in the database
    Job &job = jobList[currentIndex];
    dbManager->updateJobPreferenceScore(job.id, score);
    job.preferenceScore = score;

    // Optionally, update the UI or provide feedback to the user
    qDebug() << "Updated preference score for job ID" << job.id << "to" << score;
}

