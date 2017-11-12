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
  ui->addReviewDirButton->setIcon(add_icon);
  ui->removeReviewDirButton->setIcon(del_icon);

  connect(ui->addPaperDirButton,     &QToolButton::clicked, this, &SettingsDialog::AddPapersPath);
  connect(ui->removePaperDirButton,  &QToolButton::clicked, this, &SettingsDialog::RemoveSelectedPapersPath);
  connect(ui->addReviewDirButton,    &QToolButton::clicked, this, &SettingsDialog::AddReviewsPath);
  connect(ui->removeReviewDirButton, &QToolButton::clicked, this, &SettingsDialog::RemoveSelectedReviewsPath);
}

void SettingsDialog::SetPapersPaths(const QStringList &papers)
{
  ui->paperPathsList->clear();
  ui->paperPathsList->addItems(papers);
}

void SettingsDialog::SetReviewsPaths(const QStringList &reviews)
{
  ui->reviewsPathsList->clear();
  ui->reviewsPathsList->addItems(reviews);

  ui->chosenReviewPathCombo->addItems(reviews);
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

QStringList SettingsDialog::GetReviewsPaths() const
{
  QStringList result;

  for(int r = 0; r < ui->reviewsPathsList->count(); r++)
  {
    result << ui->reviewsPathsList->item(r)->text();
  }

  return(result);
}

void SettingsDialog::SetChosenReviewPath(const QString &dest)
{
  // Find index that matches dest

  int matched_index = -1;

  for(int i = 0; i < ui->chosenReviewPathCombo->count(); i++)
  {
    if(ui->chosenReviewPathCombo->itemText(i) == dest)
    {
      matched_index = i;
      break;
    }
  }

  if(matched_index > -1)
    ui->chosenReviewPathCombo->setCurrentIndex(matched_index);
}

QString SettingsDialog::GetChosenReviewPath() const
{
  return(ui->chosenReviewPathCombo->currentText());
}

void SettingsDialog::SetChosenReviewPathIndex(int index)
{
  ui->chosenReviewPathCombo->setCurrentIndex(index);
}

int SettingsDialog::GetChosenReviewPathIndex() const
{
  return(ui->chosenReviewPathCombo->currentIndex());
}

void SettingsDialog::AddPapersPath()
{
  QString new_path = QFileDialog::getExistingDirectory(this, tr("Add paper path"));
  if(new_path.isEmpty()) return;
  ui->paperPathsList->addItem(new_path);
  pathsChanged = true;
}

void SettingsDialog::AddReviewsPath()
{
  QString new_path = QFileDialog::getExistingDirectory(this, tr("Add review path"));
  if(new_path.isEmpty()) return;
  ui->reviewsPathsList->addItem(new_path);

  updateReviewPathChooser();

  pathsChanged = true;
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

void SettingsDialog::RemoveSelectedReviewsPath()
{
  QList<QListWidgetItem *> selected_list = ui->reviewsPathsList->selectedItems();

  for(int r = 0; r < selected_list.count(); r++)
  {
    ui->reviewsPathsList->takeItem(r);
    delete selected_list[r];
  }

  updateReviewPathChooser();

  pathsChanged = true;
}

// Scan reviewsPathsList and update chosenReviewPathCombo
void SettingsDialog::updateReviewPathChooser()
{
  QString chosen_path = ui->chosenReviewPathCombo->currentText();
  int chosen_index = -1;
  ui->chosenReviewPathCombo->clear();

  for(int r = 0; r < ui->reviewsPathsList->count(); r++)
  {
    ui->chosenReviewPathCombo->addItem(ui->reviewsPathsList->item(r)->text());
    if(chosen_path == ui->reviewsPathsList->item(r)->text())
    {
      chosen_index = r;
    }
  }

  if(chosen_index > -1) ui->chosenReviewPathCombo->setCurrentIndex(chosen_index);
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

void SettingsDialog::SetDocViewerCLI(const QString &command)
{
  ui->wordViewerEdit->setText(command);
}

QString SettingsDialog::GetDocViewerCLI() const
{
  return(ui->wordViewerEdit->text());
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
