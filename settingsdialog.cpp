/**
 * @file   settingdialog.cpp
 * @brief  Handle ReferenceOrganiser settings
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#include "settingsdialog.h"
#include "ui_settings.h"

#include <QFileDialog>
#include <QPixmap>

#include <iostream>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)

{
  ui->setupUi(this);
  pathsChanged = false;

  QIcon add_icon, del_icon;

  QPixmap add_pix1(":Assets/add.png");
  QPixmap add_pix2(":Assets/add@2x.png");
  add_pix2.setDevicePixelRatio(2.0);
  add_icon.addPixmap(add_pix1);
  add_icon.addPixmap(add_pix2);

  QPixmap del_pix1(":Assets/remove.png");
  QPixmap del_pix2(":Assets/remove@2x.png");
  del_pix2.setDevicePixelRatio(2.0);
  del_icon.addPixmap(del_pix1);
  del_icon.addPixmap(del_pix2);

  ui->addPaperDirButton->setIcon(add_icon);
  ui->removePaperDirButton->setIcon(del_icon);

  connect(ui->addPaperDirButton,         &QToolButton::released, this, &SettingsDialog::AddPapersPath);
  connect(ui->removePaperDirButton,      &QToolButton::released, this, &SettingsDialog::RemoveSelectedPapersPath);
  connect(ui->selectReadPaperPathButton, &QToolButton::released, this, &SettingsDialog::SelectReadPapersPath);
}

void SettingsDialog::SetPapersPaths(const QStringList &papers)
{
  ui->paperPathsList->clear();
  ui->paperPathsList->addItems(papers);
}

void SettingsDialog::SetReadPapersPath(const QString &read_papers)
{
  ui->readPapersPathEdit->setText(read_papers);
}

QStringList SettingsDialog::GetPapersPaths() const
{
  QStringList result;

  for(int r = 0; r < ui->paperPathsList->count(); r++)
  {
    result << ui->paperPathsList->item(r)->text();
  }

  return(result);
}

QString SettingsDialog::GetReadPapersPath() const
{
  return(ui->readPapersPathEdit->text());
}

void SettingsDialog::AddPapersPath()
{
  QString new_path = QFileDialog::getExistingDirectory(this, tr("Add paper path"));
  if(new_path.isEmpty()) return;
  ui->paperPathsList->addItem(new_path);
  pathsChanged = true;
}

void SettingsDialog::SelectReadPapersPath()
{
  QString new_path = QFileDialog::getExistingDirectory(this, tr("Select read papers path"));
  if(new_path.isEmpty()) return;
  ui->readPapersPathEdit->setText(new_path);
}

void SettingsDialog::RemoveSelectedPapersPath()
{
  QList<QListWidgetItem *> selected_list = ui->paperPathsList->selectedItems();

  for(int r = 0; r < selected_list.count(); r++)
  {
    ui->paperPathsList->takeItem(r);
    delete selected_list[r];
  }
  pathsChanged = true;
}

void SettingsDialog::SetMaxCitationAuthors(int max)
{
  ui->maxAuthorsSpin->setValue(max);
}

int SettingsDialog::GetMaxCitationAuthors() const
{
  return(ui->maxAuthorsSpin->value());
}

void SettingsDialog::SetMaxCitationCharacters(int max)
{
  ui->maxCharactersSpin->setValue(max);
}

int SettingsDialog::GetMaxCitationCharacters() const
{
  return(ui->maxCharactersSpin->value());
}

void SettingsDialog::SetDocViewerCLI(const QString &command)
{
  ui->wordViewerEdit->setText(command);
}

QString SettingsDialog::GetDocViewerCLI() const
{
  return(ui->wordViewerEdit->text());
}

void SettingsDialog::SetODTViewerCLI(const QString &command)
{
  ui->libreViewerEdit->setText(command);
}

QString SettingsDialog::GetODTViewerCLI() const
{
  return(ui->libreViewerEdit->text());
}

void SettingsDialog::SetPDFViewerCLI(const QString &command)
{
  ui->pdfViewerEdit->setText(command);
}

QString SettingsDialog::GetPDFViewerCLI() const
{
  return(ui->pdfViewerEdit->text());
}

void SettingsDialog::SetPSViewerCLI(const QString &command)
{
  ui->psViewerEdit->setText(command);
}

QString SettingsDialog::GetPSViewerCLI() const
{
  return(ui->psViewerEdit->text());
}

void SettingsDialog::SetTextViewerCLI(const QString &command)
{
  ui->textViewerEdit->setText(command);
}

QString SettingsDialog::GetTextViewerCLI() const
{
  return(ui->textViewerEdit->text());
}
void SettingsDialog::SetBackupViewerCLI(const QString &command)
{
  ui->fallbackViewerEdit->setText(command);
}

QString SettingsDialog::GetBackupViewerCLI() const
{
  return(ui->fallbackViewerEdit->text());
}
