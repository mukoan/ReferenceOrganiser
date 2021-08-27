/**
 * @file   databasemanager.h
 * @brief  Handle loading and saving multiple databases
 * @author Lyndon Hill
 * @date   2019.02.27
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QDialog>
#include <QStringList>

#include "ui_databasemanager.h"

namespace Ui {
  class DatabaseManagerDialog;
}

/**
 * @class DatabaseManager
 * @brief Handles database file selection
 */
class DatabaseManager : public QDialog
{
  Q_OBJECT

public:
  /// Cnstructor
  explicit DatabaseManager(QWidget *parent = nullptr);

  /// Destructor: closes all open databases
  ~DatabaseManager();

  /// Set the history widget
  void SetPreviousDatabases(const QStringList &names, const QStringList &filenames);

private slots:
  /// User clicked the file select button
  void chooseNewDatabaseFile();

  /// Start editing the database
  void startEditing();

  /// Load the previous database, selected from the previousDatabaseList table
  void loadPrevious();

signals:
  void edit(QString new_filename, QString new_name);

  void goToPrevious(QString filename);

private:
  Ui::DatabaseManagerDialog *ui;

  QStringList previousFilenames;
};

#endif  // DATABASEMANAGER_H
