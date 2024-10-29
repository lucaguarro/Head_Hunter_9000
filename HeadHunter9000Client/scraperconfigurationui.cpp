#include "scraperconfigurationui.h"

// Include necessary Qt modules
#include <QFile>
#include <QCryptographicHash>

ScraperConfigurationUI::ScraperConfigurationUI(QWidget *parent, QSettings *settings)
    : QWidget(parent)
    , settings(settings)
{
    // Determine the configuration file path
    configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.ini";

    // Ensure the directory exists
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));

    // Create the main layout
    mainLayout = new QVBoxLayout(this);

    // Create UI groups
    createLoginGroup();
    createScraperGroup();
    createSearchFiltersGroup();
    createDatabaseGroup();

    // Save Button
    QPushButton *saveButton = new QPushButton(tr("Save Configuration"), this);
    connect(saveButton, &QPushButton::clicked, this, &ScraperConfigurationUI::saveConfig);
    mainLayout->addWidget(saveButton);

    // Load existing configuration
    loadConfig();

    // Set the main layout
    setLayout(mainLayout);
}

ScraperConfigurationUI::~ScraperConfigurationUI()
{}

void ScraperConfigurationUI::createLoginGroup()
{
    QGroupBox *loginGroup = new QGroupBox(tr("Login"), this);
    QFormLayout *layout = new QFormLayout(loginGroup);

    emailLineEdit = new QLineEdit(loginGroup);
    layout->addRow(tr("Email:"), emailLineEdit);

    passwordLineEdit = new QLineEdit(loginGroup);
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    layout->addRow(tr("Password:"), passwordLineEdit);

    loginGroup->setLayout(layout);
    mainLayout->addWidget(loginGroup);
}

void ScraperConfigurationUI::createScraperGroup()
{
    QGroupBox *scraperGroup = new QGroupBox(tr("Scraper"), this);
    QHBoxLayout *layout = new QHBoxLayout(scraperGroup);

    chromedriverPathLineEdit = new QLineEdit(scraperGroup);
    QPushButton *browseButton = new QPushButton(tr("Browse"), scraperGroup);
    connect(browseButton, &QPushButton::clicked, this, &ScraperConfigurationUI::browseChromedriverPath);

    layout->addWidget(new QLabel(tr("Chromedriver Path:"), scraperGroup));
    layout->addWidget(chromedriverPathLineEdit);
    layout->addWidget(browseButton);

    scraperGroup->setLayout(layout);
    mainLayout->addWidget(scraperGroup);
}

void ScraperConfigurationUI::createSearchFiltersGroup()
{
    QGroupBox *filtersGroup = new QGroupBox(tr("Search Filters"), this);
    QVBoxLayout *layout = new QVBoxLayout(filtersGroup);

    // Experience Level Checkboxes
    QGroupBox *expLevelGroup = new QGroupBox(tr("Experience Level"), filtersGroup);
    QGridLayout *expLevelLayout = new QGridLayout(expLevelGroup);

    QStringList expLevelLabels = {
        "Internship",       // +1
        "Entry Level",      // +2
        "Associate",        // +4
        "Mid-Senior Level", // +8
        "Director",         // +16
        "Executive"         // +32
    };

    for (int i = 0; i < 6; ++i) {
        expLevelCheckBoxes[i] = new QCheckBox(expLevelLabels[i], expLevelGroup);
        // Arrange in 3 columns
        int row = i / 4;
        int col = i % 4;
        expLevelLayout->addWidget(expLevelCheckBoxes[i], row, col);
    }

    expLevelGroup->setLayout(expLevelLayout);
    layout->addWidget(expLevelGroup);

    // Job Type Checkboxes
    QGroupBox *jobTypeGroup = new QGroupBox(tr("Job Type"), filtersGroup);
    QGridLayout *jobTypeLayout = new QGridLayout(jobTypeGroup);

    QStringList jobTypeLabels = {
        "Full-time",   // +1
        "Part-time",   // +2
        "Contract",    // +4
        "Temp",        // +8
        "Volunteer",   // +16
        "Internship",  // +32
        "Other"        // +64
    };

    for (int i = 0; i < 7; ++i) {
        jobTypeCheckBoxes[i] = new QCheckBox(jobTypeLabels[i], jobTypeGroup);
        // Arrange in 3 columns
        int row = i / 4;
        int col = i % 4;
        jobTypeLayout->addWidget(jobTypeCheckBoxes[i], row, col);
    }

    jobTypeGroup->setLayout(jobTypeLayout);
    layout->addWidget(jobTypeGroup);

    // On-Site/Remote Checkboxes
    QGroupBox *onSiteRemoteGroup = new QGroupBox(tr("Work Arrangement"), filtersGroup);
    QGridLayout *onSiteRemoteLayout = new QGridLayout(onSiteRemoteGroup);

    QStringList onSiteRemoteLabels = {
        "On-site",   // +1
        "Remote",    // +2
        "Hybrid"     // +4
    };

    for (int i = 0; i < 3; ++i) {
        onSiteRemoteCheckBoxes[i] = new QCheckBox(onSiteRemoteLabels[i], onSiteRemoteGroup);
        onSiteRemoteLayout->addWidget(onSiteRemoteCheckBoxes[i], 0, i);
    }

    onSiteRemoteGroup->setLayout(onSiteRemoteLayout);
    layout->addWidget(onSiteRemoteGroup);

    // Date Posted
    QFormLayout *formLayout = new QFormLayout();

    datePostedComboBox = new QComboBox(filtersGroup);
    datePostedComboBox->addItem("Any Time", 1);
    datePostedComboBox->addItem("Past Month", 2);
    datePostedComboBox->addItem("Past Week", 3);
    datePostedComboBox->addItem("Past 24 Hours", 4);
    formLayout->addRow(tr("Date Posted:"), datePostedComboBox);

    // Job Title
    jobTitleLineEdit = new QLineEdit(filtersGroup);
    formLayout->addRow(tr("Job Title:"), jobTitleLineEdit);

    // Location
    locationLineEdit = new QLineEdit(filtersGroup);
    formLayout->addRow(tr("Location:"), locationLineEdit);

    layout->addLayout(formLayout);

    filtersGroup->setLayout(layout);
    mainLayout->addWidget(filtersGroup);
}

void ScraperConfigurationUI::createDatabaseGroup()
{
    QGroupBox *databaseGroup = new QGroupBox(tr("Database"), this);
    QHBoxLayout *layout = new QHBoxLayout(databaseGroup);

    dbFilePathLineEdit = new QLineEdit(databaseGroup);
    QPushButton *browseButton = new QPushButton(tr("Browse"), databaseGroup);
    connect(browseButton, &QPushButton::clicked, this, &ScraperConfigurationUI::browseDatabasePath);

    layout->addWidget(new QLabel(tr("Database File Path:"), databaseGroup));
    layout->addWidget(dbFilePathLineEdit);
    layout->addWidget(browseButton);

    databaseGroup->setLayout(layout);
    mainLayout->addWidget(databaseGroup);
}

void ScraperConfigurationUI::loadConfig()
{
    // Load LOGIN settings
    emailLineEdit->setText(settings->value("LOGIN/email", "").toString());

    // Decrypt the password (if encryption was used)
    QByteArray encryptedPassword = settings->value("LOGIN/password", "").toByteArray();
    QByteArray passwordBytes = QByteArray::fromBase64(encryptedPassword);
    passwordLineEdit->setText(QString::fromUtf8(passwordBytes));

    // Load SCRAPER settings
    chromedriverPathLineEdit->setText(settings->value("SCRAPER/chromedriver_filepath", "").toString());

    // Load SEARCH_FILTERS settings

    // Load and set exp_level checkboxes
    int expLevelValue = settings->value("SEARCH_FILTERS/exp_level", 0).toInt();
    for (int i = 0; i < 6; ++i) {
        int checkboxValue = 1 << i; // 2^i
        bool checked = (expLevelValue & checkboxValue) != 0;
        expLevelCheckBoxes[i]->setChecked(checked);
    }

    // Load and set job_type checkboxes
    int jobTypeValue = settings->value("SEARCH_FILTERS/job_type", 0).toInt();
    for (int i = 0; i < 7; ++i) {
        int checkboxValue = 1 << i; // 2^i
        bool checked = (jobTypeValue & checkboxValue) != 0;
        jobTypeCheckBoxes[i]->setChecked(checked);
    }

    // Load and set on_site_remote checkboxes
    int onSiteRemoteValue = settings->value("SEARCH_FILTERS/on_site_remote", 0).toInt();
    for (int i = 0; i < 3; ++i) {
        int checkboxValue = 1 << i; // 2^i
        bool checked = (onSiteRemoteValue & checkboxValue) != 0;
        onSiteRemoteCheckBoxes[i]->setChecked(checked);
    }

    // Date Posted
    int datePostedValue = settings->value("SEARCH_FILTERS/date_posted", 1).toInt();
    int datePostedIndex = datePostedComboBox->findData(datePostedValue);
    if (datePostedIndex != -1) {
        datePostedComboBox->setCurrentIndex(datePostedIndex);
    }

    // Job Title
    jobTitleLineEdit->setText(settings->value("SEARCH_FILTERS/job_title", "").toString());

    // Location
    locationLineEdit->setText(settings->value("SEARCH_FILTERS/location", "").toString());

    // Load DATABASE settings
    dbFilePathLineEdit->setText(settings->value("DATABASE/db_filepath", "").toString());
}

void ScraperConfigurationUI::saveConfig()
{
    // Save LOGIN settings
    settings->setValue("LOGIN/email", emailLineEdit->text());

    // Encrypt the password (simple base64 encoding; consider stronger encryption in production)
    QByteArray passwordBytes = passwordLineEdit->text().toUtf8();
    QByteArray encryptedPassword = passwordBytes.toBase64();
    settings->setValue("LOGIN/password", encryptedPassword);

    // Save SCRAPER settings
    settings->setValue("SCRAPER/chromedriver_filepath", chromedriverPathLineEdit->text());

    // Save SEARCH_FILTERS settings

    // Calculate exp_level value from checkboxes
    int expLevelValue = 0;
    for (int i = 0; i < 6; ++i) {
        if (expLevelCheckBoxes[i]->isChecked()) {
            expLevelValue += (1 << i); // 2^i
        }
    }
    settings->setValue("SEARCH_FILTERS/exp_level", expLevelValue);

    // Calculate job_type value from checkboxes
    int jobTypeValue = 0;
    for (int i = 0; i < 7; ++i) {
        if (jobTypeCheckBoxes[i]->isChecked()) {
            jobTypeValue += (1 << i); // 2^i
        }
    }
    settings->setValue("SEARCH_FILTERS/job_type", jobTypeValue);

    // Calculate on_site_remote value from checkboxes
    int onSiteRemoteValue = 0;
    for (int i = 0; i < 3; ++i) {
        if (onSiteRemoteCheckBoxes[i]->isChecked()) {
            onSiteRemoteValue += (1 << i); // 2^i
        }
    }
    settings->setValue("SEARCH_FILTERS/on_site_remote", onSiteRemoteValue);

    // Date Posted
    settings->setValue("SEARCH_FILTERS/date_posted", datePostedComboBox->currentData().toInt());

    // Job Title
    settings->setValue("SEARCH_FILTERS/job_title", jobTitleLineEdit->text());

    // Location
    settings->setValue("SEARCH_FILTERS/location", locationLineEdit->text());

    // Save DATABASE settings
    settings->setValue("DATABASE/db_filepath", dbFilePathLineEdit->text());
    emit databasePathChanged();

    // Ensure settings are written to file
    settings->sync();

    QMessageBox::information(this, tr("Configuration Saved"), tr("Configuration has been saved successfully."));
}

void ScraperConfigurationUI::browseChromedriverPath()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select Chromedriver"), QDir::homePath());
    if (!filePath.isEmpty()) {
        chromedriverPathLineEdit->setText(QDir::toNativeSeparators(filePath));
    }
}

void ScraperConfigurationUI::browseDatabasePath()
{
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Select or Create Database File"));
    dialog.setDirectory(QDir::homePath());
    dialog.setNameFilter(tr("Database Files (*.db)"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setOption(QFileDialog::DontConfirmOverwrite, true);

    if (dialog.exec() == QDialog::Accepted) {
        QString filePath = dialog.selectedFiles().first();
        // Ensure the file has a '.db' extension
        if (!filePath.endsWith(".db", Qt::CaseInsensitive)) {
            filePath += ".db";
        }
        dbFilePathLineEdit->setText(QDir::toNativeSeparators(filePath));
    }
}

