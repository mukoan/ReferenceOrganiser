/**
 * @file   createdatabasedialog.cpp
 * @brief  Get details to create new database
 * @author Lyndon Hill
 * @date   2019.02.27
 */

#include "createdatabasedialog.h"


#include <QStandardPaths>
#include <QFileDialog>
#include <QPushButton>

CreateDatabaseDialog::CreateDatabaseDialog(bool answer_required, QWidget *parent) : QDialog(parent), ui(new Ui::CreateDatabaseDialog)
{
  ui->setupUi(this);

  if(answer_required) {
    QPushButton *reject_button = ui->buttonBox->button(QDialogButtonBox::Cancel);
    reject_button->setText(tr("Quit"));
  }

  connect(ui->actionCombo,              &QComboBox::currentIndexChanged, this, &CreateDatabaseDialog::respondModeChange);
  connect(ui->databaseFileSelectButton, &QToolButton::released,          this, &CreateDatabaseDialog::chooseDatabaseFile);
  connect(ui->buttonBox,                &QDialogButtonBox::accepted,     this, &CreateDatabaseDialog::accept);
  connect(ui->buttonBox,                &QDialogButtonBox::rejected,     this, &CreateDatabaseDialog::reject);
}

CreateDatabaseDialog::~CreateDatabaseDialog()
{
  delete ui;
}

// Get name of new database
QString CreateDatabaseDialog::GetDatabaseName() const
{
  return(ui->databaseNameEdit->text());
}

// Get filename of new database
QString CreateDatabaseDialog::GetDatabaseFilename() const
{
  return(ui->databaseFilenameEdit->text());
}

// User has selected to create a database
bool CreateDatabaseDialog::IsCreateMode() const
{
  return(ui->actionCombo->currentIndex() == 0);
}

// Mode combo changed
void CreateDatabaseDialog::respondModeChange(int new_mode)
{
  switch(new_mode)
  {
    case 0:
    // create
    ui->databaseNameEdit->setEnabled(true);
    break;

    case 1:
    // load existing
    ui->databaseNameEdit->setEnabled(false);
    break;
  }
}

// User clicked the file select button
void CreateDatabaseDialog::chooseDatabaseFile()
{
  QString documents_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  if(documents_path.isEmpty()) documents_path = QDir::homePath();

  // Get filename to save to
  QString filename;

  switch(ui->actionCombo->currentIndex())
  {
    case 0:
    // create
    filename = QFileDialog::getSaveFileName(this, tr("Database filename"),
                                            QString("%1/papers.rodb").arg(documents_path),
                                            tr("Database files (*.rodb *.xml)"));
    break;

    case 1:
    // load existing
    filename = QFileDialog::getOpenFileName(this, tr("Database filename"), documents_path, tr("Database files (*.rodb *.xml)"));
    break;
  }

  if(filename.isEmpty())
    return;

  ui->databaseFilenameEdit->setText(filename);
}
