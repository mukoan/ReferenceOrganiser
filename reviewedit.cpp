/**
 * @file   reviewedit.cpp
 * @brief  Edit/create a review
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#include <iostream>
#include <fstream>

#include <QStringList>
#include <QFile>
#include <QDir>
#include <QMessageBox>

#include "reviewedit.h"
#include "reviewparser.h"


ReviewEdit::ReviewEdit(bool create, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReviewEditDialog), createMode(create), hasChanged(false)
{
  ui->setupUi(this);

  maxAuthors    = 5;
  maxCiteLength = 32;
  records       = nullptr;

  if(!createMode)
    ui->saveButton->setEnabled(true);
  else
    ui->saveButton->setEnabled(false);

  ui->generateButton->setEnabled(false);
  ui->generateButton->setToolTip(tr("Enter authors and year"));

  connect(ui->citationEdit,   &QLineEdit::textChanged, this, &ReviewEdit::checkCitation);
  connect(ui->generateButton, &QPushButton::released,  this, &ReviewEdit::generateKey);
  connect(ui->saveButton,     &QPushButton::released,  this, &ReviewEdit::preSave);
  connect(ui->cancelButton,   &QPushButton::released,  this, &ReviewEdit::endEdit);

  connect(ui->yearEdit,       &QLineEdit::textChanged, this, &ReviewEdit::checkGeneratePossible);
  connect(ui->authorsEdit,    &QLineEdit::textChanged, this, &ReviewEdit::checkGeneratePossible);
  connect(ui->reviewEdit,     &QPlainTextEdit::textChanged, this, &ReviewEdit::reviewChanged);
}

void ReviewEdit::SetCitationKey(const QString &cite)
{
  originalCitation = cite;
  ui->citationEdit->setText(cite);

  // ui->saveButton->setEnabled(!createMode);
}

void ReviewEdit::SetAuthors(const QString &authors)
{
  ui->authorsEdit->setText(authors);
}

void ReviewEdit::SetTitle(const QString &title)
{
  ui->titleEdit->setText(title);
}

void ReviewEdit::SetYear(const QString &year)
{
  ui->yearEdit->setText(year);
}

void ReviewEdit::SetReviewText(const QString &text)
{
  ui->reviewEdit->setPlainText(text);
}

void ReviewEdit::SetRecords(const QVector<PaperRecord> &rec)
{
  records = &rec;
}

// Generate citation key based on author names and year
void ReviewEdit::generateKey()
{
  QString key;
  QString cat_authors;

  // Extract surnames and concatenate
  // Note: this won't do a good job for peopple with multiple surnames unless they are hyphenated
  QStringList names = ui->authorsEdit->text().split(',', QString::SkipEmptyParts);
  if(names.empty()) return;

  if(names.size() <= maxAuthors)
  {
    for(int n = 0; n < names.size(); n++)
    {
      QStringList name_elements = names[n].split(' ', QString::SkipEmptyParts); // assume family name last
      cat_authors.append(name_elements.last());
    }
    key = cat_authors + ui->yearEdit->text();
  }

  if((names.size() > maxAuthors) || (key.size() > maxCiteLength))
  {
    // Single author name in case of too many authors or too long string
    QStringList name_elements = names[0].split(' ', QString::SkipEmptyParts); // assume family name last

    key = name_elements.last() + tr("EtAl") + ui->yearEdit->text();

    if(!checkCitationHasReview(key))
    {
      // -> Try AlphaBravoCharlie2000 = ABC2000
      key.clear();
      for(int a = 0; a < names.size(); a++)
      {
        QStringList name_elements = names[a].split(' ', QString::SkipEmptyParts); // assume family name last
        key.append(name_elements.last()[0]);
      }
      key = key + ui->yearEdit->text();
    }
  }

  // Check if key already exists in records

  bool unique = checkCitationHasReview(key);

  // -> Add a, b, c etc to key
  if(!unique)
  {
    if(checkCitationHasReview(key + "a"))
    {
      key = key + "a";
    }
    else if(checkCitationHasReview(key + "b"))
    {
      key = key + "b";
    }
    else if(checkCitationHasReview(key + "c"))
    {
      key = key + "c";
    }
    else
    {
      // TODO: incorporate title/venue in key...
    }
  }

  ui->citationEdit->setText(key);
}

// Checks if review exists for citation
bool ReviewEdit::checkCitationHasReview(const QString &text)
{
  if(!records) return(false);

  bool exists = false;

  int i;
  for(i = 0; i < records->size(); i++)
  {
    if((*records)[i].citation == text)
    {
      if(!(*records)[i].reviewPath.isEmpty())
        exists = true;
      break;
    }
  }

  return(!exists);
}

QString ReviewEdit::GetCitationKey() const
{
  return(ui->citationEdit->text());
}

QString ReviewEdit::GetAuthors() const
{
  return(ui->authorsEdit->text());
}

QString ReviewEdit::GetTitle() const
{
  return(ui->titleEdit->text());
}

QString ReviewEdit::GetYear() const
{
  return(ui->yearEdit->text());
}

QString ReviewEdit::GetReviewText() const
{
  return(ui->reviewEdit->toPlainText());
}

// Set save button enabled only if citation text edit has something
void ReviewEdit::checkCitation(const QString &text)
{  
  if(!createMode && (text == originalCitation))
    ui->saveButton->setEnabled(true);
  else
    ui->saveButton->setEnabled(!text.isEmpty() && checkCitationHasReview(text));

  hasChanged = true;
}

// Only possible if authors and year set
void ReviewEdit::checkGeneratePossible(const QString &)
{
  bool possible = (!ui->authorsEdit->text().isEmpty()) && (!ui->yearEdit->text().isEmpty());
  ui->generateButton->setEnabled(possible);

  if(possible)
    ui->generateButton->setToolTip(tr("Generate citation key from authors and year"));
  else
    ui->generateButton->setToolTip(tr("Enter authors and year"));

  hasChanged = true;
}

// The review text changed
void ReviewEdit::reviewChanged()
{
  hasChanged = true;
}

// Check review is suitable for saving and save if it is
void ReviewEdit::preSave()
{
  bool file_moved = false;
  QString dest_file;

  if(createMode)
  {
    // Check if unique citation
    if(!checkCitationHasReview(ui->citationEdit->text()))
    {
      QMessageBox::warning(this, tr("Citation in Use"), tr("The citation %1 is already being used for another paper. Choose another citation").arg(ui->citationEdit->text()));
      return; // don't save
    }

    dest_file = targetDirectory + QDir::separator() + ui->citationEdit->text();
  }
  else
  {
    // Check if citation was changed; if so delete the old file
    if(originalCitation != ui->citationEdit->text())
    {
      file_moved = true;
      dest_file = targetDirectory + QDir::separator() + ui->citationEdit->text();
    }
    else
      dest_file = reviewPath;
  }

  // Get year from citation key and check against year edit
  QString citation_year;
  ParseYearFromCitation(ui->citationEdit->text(), citation_year);
  {
    QString edit_year = ui->yearEdit->text();
    if(!edit_year.isEmpty() && (edit_year != citation_year))
    {
      QMessageBox::warning(this, tr("Year Problem"), tr("In this version the year is stored in the citation key and must match the year value. Modify the citation or clear the year value."));
      return; // don't save
    }
  }

  if(saveReview(dest_file, ui->titleEdit->text(), ui->authorsEdit->text(), ui->reviewEdit->toPlainText()))
  {
    if(file_moved)
    {
      QString old_filename = reviewPath;
      QFile::remove(old_filename);
    }

    emit reviewSaved(createMode | file_moved);

    close();
  }
  else
  {
    // Error message
    QMessageBox::warning(this, tr("Review could not be saved"), tr("There was an error saving the review"));
  }
}

// Close, but check saved first
void ReviewEdit::endEdit()
{
  bool check_with_user = false;
  QString title_text, message_text;

  // Check if there is a review that should be saved
  if(createMode)
  {
    // Warn if any details filled in that haven't been saved
    if((!ui->titleEdit->text().isEmpty()) || (!ui->authorsEdit->text().isEmpty()) || (!ui->reviewEdit->toPlainText().isEmpty()))
    {
      check_with_user = true;
      title_text      = tr("Unsaved Review");
      message_text    = tr("There is unsaved data, are you sure you want to discard it?");
    }
  }
  else if(hasChanged)
  {
    check_with_user = true;
    title_text      = tr("Unsaved Changes");
    message_text    = tr("There are unsaved changes, are you sure you want to discard them?");
  }

  if(check_with_user)
  {
    QMessageBox unsaved(QMessageBox::Warning, title_text, message_text,
                        QMessageBox::Cancel | QMessageBox::Discard,
                        this);

    if(unsaved.exec() == QMessageBox::Cancel)
      return; // abort close

  }

  close();
}

// Save a review
bool ReviewEdit::saveReview(const QString &path, const QString &title, const QString &authors, const QString &text)
{
  std::ofstream op(path.toUtf8().constData());
  if(op.bad())
  {
    std::cerr << "Error: Could not write review\n";
    return(false);
  }

  op << title.toStdString() << "\n";
  op << authors.toStdString() << "\n\n";
  op << text.toStdString() << "\n";
  op.close();

  // TODO Check for failures when writing to output stream

  return(true);
}
