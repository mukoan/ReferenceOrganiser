/**
 * @file   duplicatesviewer.cpp
 * @brief  Show similar records
 * @author Lyndon Hill
 * @date   2024.07.16
 */


#include "duplicatesviewer.h"

DuplicatesViewer::DuplicatesViewer(QWidget *parent)
    : QDialog{parent}, ui(new Ui::DuplicatesDialog)
{
  ui->setupUi(this);

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DuplicatesViewer::accept);
}


// Set duplicates to show
void DuplicatesViewer::SetDuplicates(const QVector<PaperMeta> &duplicate_list)
{
  ui->duplicatesTable->setRowCount(duplicate_list.size());

  for(int p = 0; p < duplicate_list.size(); p++) {
    ui->duplicatesTable->setItem(p, 0, new QTableWidgetItem(duplicate_list[p].citation));
    ui->duplicatesTable->setItem(p, 1, new QTableWidgetItem(duplicate_list[p].authors));
    ui->duplicatesTable->setItem(p, 2, new QTableWidgetItem(duplicate_list[p].title));
    ui->duplicatesTable->setItem(p, 3, new QTableWidgetItem(duplicate_list[p].year));
  }

  ui->duplicatesTable->resizeColumnsToContents();
  ui->duplicatesTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
}
