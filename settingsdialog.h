/**
 * @file   settingdialog.h
 * @brief  Handle ReferenceOrganiser settings
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
  class SettingsDialog;
}


class SettingsDialog : public QDialog
{
  Q_OBJECT

public:
  /// Constructor
  SettingsDialog(QWidget *parent = 0);

  void SetPapersPaths(const QStringList &papers);
  void SetReviewsPaths(const QStringList &reviews);

  void SetChosenReviewPath(const QString &dest);
  QString GetChosenReviewPath() const;

  void SetChosenReviewPathIndex(int index);
  int GetChosenReviewPathIndex() const;

  QStringList GetPapersPaths() const;
  QStringList GetReviewsPaths() const;

  void SetMaxCitationAuthors(int max);
  int GetMaxCitationAuthors() const;

  void SetMaxCitationCharacters(int max);
  int GetMaxCitationCharacters() const;

  void SetPDFViewerCLI(const QString &command);
  QString GetPDFViewerCLI() const;

  void SetPSViewerCLI(const QString &command);
  QString GetPSViewerCLI() const;

  void SetDocViewerCLI(const QString &command);
  QString GetDocViewerCLI() const;

  void SetTextViewerCLI(const QString &command);
  QString GetTextViewerCLI() const;

  void SetBackupViewerCLI(const QString &command);
  QString GetBackupViewerCLI() const;

  bool GetPathsChanged() const { return(pathsChanged); }

private slots:
  void AddPapersPath();
  void AddReviewsPath();

  void RemoveSelectedPapersPath();
  void RemoveSelectedReviewsPath();

private:
  /// Scan reviewsPathsList and update chosenReviewPathCombo
  void updateReviewPathChooser();

  Ui::SettingsDialog *ui;

  /// Search paths were edited
  bool pathsChanged;
};

#endif // SETTINGSDIALOG_H
