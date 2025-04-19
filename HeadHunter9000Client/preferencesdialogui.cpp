#include "preferencesdialogui.h"
#include "llmconfigpage.h"
#include "resumetemplatepage.h"
#include "coverlettertemplatepage.h"

#include <QHBoxLayout>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QHeaderView>

PreferencesDialogUI::PreferencesDialogUI(QWidget *parent, QSettings *settings)
    : QDialog(parent), settings(settings) {
    setWindowTitle("Preferences / Settings");
    resize(900, 600);
    setupUI();
}

void PreferencesDialogUI::setupUI() {
    QHBoxLayout *layout = new QHBoxLayout(this);
    navigationTree = new QTreeWidget(this);
    navigationTree->setHeaderHidden(true);
    navigationTree->setFixedWidth(200);

    setupNavigation();

    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(new QWidget());                               // Index 0 - Job Search Criteria
    stackedWidget->addWidget(new QWidget());                               // Index 1 - Database
    stackedWidget->addWidget(new LLMConfigPage(this, settings));           // Index 2 - LLM Configuration
    stackedWidget->addWidget(new ResumeTemplatePage(settings, this));      // Index 3 - Resume Template
    stackedWidget->addWidget(new CoverLetterTemplatePage(settings, this)); // Index 4 - Cover Letter Template

    layout->addWidget(navigationTree);
    layout->addWidget(stackedWidget);

    connect(navigationTree, &QTreeWidget::itemClicked, this, [=](QTreeWidgetItem *item){
        stackedWidget->setCurrentIndex(item->data(0, Qt::UserRole).toInt());
    });
}


void PreferencesDialogUI::setupNavigation() {
    QStringList topItems = {"Job Search Criteria", "Database", "AI Generated Docs"};
    for (int i = 0; i < topItems.size(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(topItems[i]));
        navigationTree->addTopLevelItem(item);

        if (topItems[i] == "AI Generated Docs") {
            QStringList subItems = {"LLM Configuration", "Resume Template", "Cover Letter Template"};
            for (int j = 0; j < subItems.size(); ++j) {
                QTreeWidgetItem *subItem = new QTreeWidgetItem(QStringList(subItems[j]));
                subItem->setData(0, Qt::UserRole, 2 + j); // Match stackedWidget index
                item->addChild(subItem);
            }
        } else {
            item->setData(0, Qt::UserRole, i);
        }
    }
}
