#ifndef SCRAPERCONFIGURATIONUI_H
#define SCRAPERCONFIGURATIONUI_H

#include <QWidget>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QCheckBox>

class ScraperConfigurationUI : public QWidget
{
    Q_OBJECT

public:
    explicit ScraperConfigurationUI(QWidget *parent = nullptr);
    ~ScraperConfigurationUI();

private slots:
    void saveConfig();
    void browseChromedriverPath();
    void browseDatabasePath();

private:
    void loadConfig();
    void createLoginGroup();
    void createScraperGroup();
    void createSearchFiltersGroup();
    void createDatabaseGroup();

    // Configuration file path
    QString configFilePath;

    // UI Elements
    QLineEdit *emailLineEdit;
    QLineEdit *passwordLineEdit;

    QLineEdit *chromedriverPathLineEdit;

    // For exp_level checkboxes
    QCheckBox *expLevelCheckBoxes[6];

    // For job_type checkboxes
    QCheckBox *jobTypeCheckBoxes[7];

    // For on_site_remote checkboxes
    QCheckBox *onSiteRemoteCheckBoxes[3];

    QComboBox *datePostedComboBox;
    QLineEdit *jobTitleLineEdit;
    QLineEdit *locationLineEdit;

    QLineEdit *dbFilePathLineEdit;

    // Layouts
    QVBoxLayout *mainLayout;
    QSettings *settings;
};

#endif // SCRAPERCONFIGURATIONUI_H
