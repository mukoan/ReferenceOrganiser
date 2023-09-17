/**
 * @file   addpaperdialog.cpp
 * @brief  Add basic paper details
 * @author Lyndon Hill
 * @date   2023.08.23
 */

#include <iostream>

#include <QMessageBox>

#include "addpaperdialog.h"

AddPaperDialog::AddPaperDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddPaperDialog)
{
  ui->setupUi(this);

  connect(ui->saveButton,     &QPushButton::released,  this, &AddPaperDialog::save);
  connect(ui->cancelButton,   &QPushButton::released,  this, &AddPaperDialog::close);
}

void AddPaperDialog::SetPaperPath(const QString &path)
{
  ui->paperPathEdit->setText(path);

  QString full_filename = path.section('/', -1);
  QString filename = full_filename.section('.', 0, -2); // Include filenames with period in them
  ui->citationEdit->setText(filename);
}

void AddPaperDialog::save()
{
  emit recordToSave(ui->citationEdit->text());
}

QString AddPaperDialog::GetCitationKey() const
{
  return(ui->citationEdit->text());
}

QString AddPaperDialog::GetAuthors() const
{
  return(ui->authorsEdit->text());
}

QString AddPaperDialog::GetTitle() const
{
  return(ui->titleEdit->text());
}

QString AddPaperDialog::GetYear() const
{
  return(ui->yearEdit->text());
}

QString AddPaperDialog::GetURL() const
{
  return(ui->urlEdit->text());
}

// Inform the dialog about the attempt to save
void AddPaperDialog::SaveResult(bool success)
{
  if(success)
    accept();
  else
    QMessageBox::warning(this, tr("Citation in Use"),
       tr("The citation %1 is already being used for another paper. Choose another citation").arg(ui->citationEdit->text()));
}

bool AddPaperDialog::MoveToStorage() const
{
  return(ui->moveToStorageCheck->isChecked());
}
