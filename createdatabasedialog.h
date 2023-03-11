/**
 * @file   createdatabasedialog.h
 * @brief  Get details to create new database
 * @author Lyndon Hill
 * @date   2019.02.27
 */

#ifndef CREATEDATABASEDIALOG_H
#define CREATEDATABASEDIALOG_H

#include <QDialog>

namespace Ui {
class CreateDatabaseDialog;
}

class CreateDatabaseDialog : public QDialog
{
  Q_OBJECT

public:
  explicit CreateDatabaseDialog(QWidget *parent = nullptr);
  ~CreateDatabaseDialog();

    /// Get name of new database
    QString GetDatabaseName() const;

    /// Get filename of new database
    QString GetDatabaseFilename() const;

    /// User has selected to create a database
    bool IsCreateMode() const;

private slots:
  /// Mode combo changed
  void respondModeChange(int new_mode);

  /// User clicked the file select button
  void chooseDatabaseFile();

private:
  Ui::CreateDatabaseDialog *ui;
};

#endif // CREATEDATABASEDIALOG_H
