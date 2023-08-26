/**
 * @file   addpaperdialog.h
 * @brief  Add basic paper details
 * @author Lyndon Hill
 * @date   2023.08.23
 */

#ifndef ADDPAPERDIALOG_H
#define ADDPAPERDIALOG_H

#include <QDialog>
#include <QVector>

#include "ui_addpaper.h"

namespace Ui {
  class AddPaperDialog;
}

/**
 * @brief Dialog for adding some simple paper details
 */
class AddPaperDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * @brief AddPaperDialog
   * @param parent  parent widget
   */
  AddPaperDialog(QWidget *parent = 0);

  void SetPaperPath(const QString &path);

  QString GetCitationKey() const;
  QString GetAuthors() const;
  QString GetTitle() const;
  QString GetYear() const;

  /// The paper should be moved to the new papers directory on acceptance
  bool MoveToStorage() const;

  /**
   * Inform the dialog about the attempt to save
   * @param success   if true, close the dialog; if false citation is already in use
   */
  void SaveResult(bool success);

signals:
  /// A record is ready to be saved
  void recordToSave(const QString &citation);

private slots:
  /// Start process to check citation and save record
  void save();

private:
  Ui::AddPaperDialog *ui;
};

#endif  // ADDPAPERDIALOG_H
