/**
 * @file   databasemanager.cpp
 * @brief  Handle loading and saving multiple databases
 * @author Lyndon Hill
 * @date   2019.02.27
 */

#include "databasemanager.h"

#include <QStandardPaths>
#include <QFileDialog>

DatabaseManager::DatabaseManager(QWidget *parent)
    : QDialog(parent), ui(new Ui::DatabaseManagerDialog)
{
  connect(ui->newFileSelectButton, &QToolButton::clicked, this, &DatabaseManager::chooseNewDatabaseFile);
  connect(ui->startNewButton,      &QPushButton::clicked, this, &DatabaseManager::startEditing);
  connect(ui->loadPreviousButton,  &QPushButton::clicked, this, &DatabaseManager::loadPrevious);
}

DatabaseManager::~DatabaseManager()
{
  delete ui;
}

// Set the history widget
void DatabaseManager::SetPreviousDatabases(const QStringList &names, const QStringList &filenames)
{
  ui->previousDatabaseList->clear();
  ui->previousDatabaseList->addItems(names);

  previousFilenames = filenames;
}

// User clicked the file select button
void DatabaseManager::chooseNewDatabaseFile()
{
  QString documents_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  if(documents_path.isEmpty()) documents_path = QDir::homePath();

  // Get filename to save to
  QString filename = QFileDialog::getSaveFileName(this, tr("Database filename"),
                                                        QString("%1/papers.rodb").arg(documents_path),
                                                        tr("Database files (*.rodb *.xml)"));
  if(filename.isEmpty())
    return;

  ui->newFileEdit->setText(filename);
}

// Start editing the database
void DatabaseManager::startEditing()
{
  emit(edit(ui->newFileEdit->text(), ui->newNameEdit->text()));
}

// Load the previous database, selected from the previousDatabaseList table
void DatabaseManager::loadPrevious()
{
  int previous_index = ui->previousDatabaseList->currentRow();
  if(previous_index < 0) return;     // FIXME what is value if no current?

  QString db_to_load = previousFilenames[previous_index];
  emit(goToPrevious(db_to_load));
  accept();
}
