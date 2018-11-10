/**
 * @file   reviewrdit.h
 * @brief  Edit/create a review
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#ifndef REVIEWEDIT_H
#define REVIEWEDIT_H

#include <QDialog>
#include <QVector>

#include "ui_newreview.h"
#include "paperrecord.h"

namespace Ui {
  class ReviewEditDialog;
}

/**
 * @brief Dialog for creating or editing a review
 */
class ReviewEdit : public QDialog
{
  Q_OBJECT

public:
  /**
   * @brief ReviewEdit
   * @param create  open in create a review mode
   * @param parent  parent widget
   */
  ReviewEdit(bool create, QWidget *parent = 0);

  void SetCitationKey(const QString &cite);
  void SetAuthors(const QString &authors);
  void SetTitle(const QString &title);
  void SetYear(const QString &year);
  void SetReviewText(const QString &text);

  QString GetCitationKey() const;
  QString GetAuthors() const;
  QString GetTitle() const;
  QString GetYear() const;
  QString GetReviewText() const;

  /// Set maximum number of authors to use in a citation
  void SetMaximumAuthors(int max)    { maxAuthors = max; }

  /// Set maximum number of characters to use in a citation
  void SetMaximumCharacters(int max) { maxCiteLength = max; }

  /// Set existing records, used when checking uniqueness of a citation
  void SetRecords(const QVector<PaperRecord> &rec);

  /// Where file is to be saved to
  void SetTargetDir(const QString &location) { targetDirectory = location; }

  /// Full path to review being edited
  void SetReviewPath(const QString &location) { reviewPath = location; }

signals:
  /// A review was saved/modified either new or citation changed thus requiring update of the GUI
  void reviewSaved(bool refresh_required);

private slots:
  /// Generate citation key based on author names and year
  void generateKey();

  /// Set save button enabled only if citation text edit has something
  /// and citation is unique/has no review
  void checkCitation(const QString &text);

  // Only possible if authors and year set
  void checkGeneratePossible(const QString &);

  // The review text changed
  void reviewChanged();

  /// Save, close window
  void saveAndClose();

  /// Save but don't close window
  void saveWithoutClosing();

  /// Close, but check saved first
  void endEdit();

private:
  /// Check review is suitable for saving and save if it is
  void preSave(bool close_window);

  /// Save the review with these details; overwrite if already exists
  bool saveReview(const QString &citation, const QString &title, const QString &authors, const QString &review);

  bool checkCitationHasReview(const QString &text);

  Ui::ReviewEditDialog *ui;

  /// The original citation
  QString originalCitation;

  /// Full path for where to save file to if create
  QString targetDirectory;

  /// Full path to file if editing
  QString reviewPath;

  /// Maximum number of authors to include in a citation
  int maxAuthors;

  /// Maximum number of characters in a citation
  int maxCiteLength;

  /// The current list of reviews and papers
  const QVector<PaperRecord> *records;

  /// The review is being created and does not exist yet
  bool createMode;

  /// The review and details have been changed
  bool hasChanged;
};

#endif // REVIEWEDIT_H
