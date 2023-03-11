/**
 * @file   databasenamedialog.h
 * @brief  Rename database
 * @author Lyndon Hill
 * @date   2021.08.21
 */


#ifndef DATABASENAMEDIALOG_H
#define DATABASENAMEDIALOG_H

#include <QDialog>

namespace Ui {
class DatabaseNameDialog;
}

class DatabaseNameDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DatabaseNameDialog(QWidget *parent = nullptr);
  ~DatabaseNameDialog();

  void SetName(const QString &name);
  QString GetName() const;

private:
  Ui::DatabaseNameDialog *ui;
};

#endif // DATABASENAMEDIALOG_H
