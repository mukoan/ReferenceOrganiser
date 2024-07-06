/**
 * @file   metadialog.h
 * @brief  Edit paper metadata
 * @author Lyndon Hill
 * @date   2020.10.11
 */


#ifndef METADIALOG_H
#define METADIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVector>

#include "papermeta.h"

namespace Ui {
  class MetaDialog;
}

/// Dialog handling all paper metadata
class MetaDialog : public QDialog
{
  Q_OBJECT

public:
  explicit MetaDialog(QWidget *parent = nullptr);
  ~MetaDialog();

  /// Set the meta data
  void SetMeta(const PaperMeta &paper_details);

  /// Get the data from the form
  PaperMeta GetMeta();

  /// Set the pre existing tags; call only once
  void SetTags(const QStringList &tags);

  /// Disable move paper option
  void SetPaperIngested();

  /// Check if paper should be moved
  bool GetIngestPaper();

  /// Where to look for papers
  void SetPaperHuntDir(const QString &path) { paperHuntDir = path; }

  /// Where to find read papers
  void SetReadPapersDir(const QString &path) { readPapersPath = path; }

  /// Set font to use in editor
  void SetEditFont(const QString &font_name, int font_size);

public slots:
  /// Set the citation: use if not using SetMeta() or used when generating citation
  void SetCitation(const QString &cite);

  /// Let the dialog know aboui possible duplicates
  void SuggestDuplicates(const QVector<PaperMeta> &duplicates);

signals:
  /// Details were updated
  void updatedDetails(PaperMeta &review);

  /// Request a citation for the given authors and year
  void requestCitation(const QString &authors, const QString &year);

  /// Check if the reference has already been added
  void checkDuplicates(const QString &authors, const QString &title, const QString &year);

  /// User found a paper
  void paperLocated(const QString &paper_path);

private slots:
  /// Venue type was changed
  void setVenueType(int);

  /// React to user requesting a citation
  void generateCite();

  /// Request meta to be saved and close dialog
  void saveAndClose();

  /// Request meta to be saved
  void saveOnly();

  /// User hit cancel or keyboard shortcut, like escape
  void requestToCancel();

  /// User to find the paper PDF or other file type
  void locatePaper();

  /// Add a tag from the tagCombo
  void addTag();

  /// Clear tag list
  void clearTags();

  /// Citation was edited, enable/disable save button(s)
  void citationUpdated(const QString &);

  /// Check if paper path is already in read papers directory
  void checkDenyMove(const QString &filepath);

protected:
  void keyPressEvent(QKeyEvent *e);

private:
  Ui::MetaDialog *ui;
  QPushButton *saveContinueButton;

  QString originalCitation; ///< Paper citation before editing
  QString paperHuntDir;
  QString readPapersPath;   ///< Where read papers are stored

  /// User to find the paper PDF or other file type, with citation to try as stem of filename
  void locatePaperByName(const QString &cite);

  /// Original copy of the metadata
  PaperMeta originalCopy;

  /// Dialog was opened to edit a review
  bool isEdit;
};

#endif  // METADIALOG_H
