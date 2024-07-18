/**
 * @file   duplicatesviewer.h
 * @brief  Show similar records
 * @author Lyndon Hill
 * @date   2024.07.16
 */

#ifndef DUPLICATESVIEWER_H
#define DUPLICATESVIEWER_H

#include <QDialog>
#include <QVector>

#include "papermeta.h"

#include "ui_duplicatesdialog.h"

namespace Ui {
  class DuplicatesDialog;
}

class DuplicatesViewer : public QDialog
{
  Q_OBJECT

public:
  explicit DuplicatesViewer(QWidget *parent = nullptr);

  /// Set duplicates to show
  void SetDuplicates(const QVector<PaperMeta> &duplicate_list);

private:
  Ui::DuplicatesDialog *ui;

};

#endif // DUPLICATESVIEWER_H
