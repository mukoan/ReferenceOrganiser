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

  void SetReadPapersPath(const QString &dest);
  QString GetReadPapersPath() const;

  QStringList GetPapersPaths() const;

  void SetMaxCitationAuthors(int max);
  int GetMaxCitationAuthors() const;

  void SetMaxCitationCharacters(int max);
  int GetMaxCitationCharacters() const;

  void SetDocViewerCLI(const QString &command);
  QString GetDocViewerCLI() const;

  void SetODTViewerCLI(const QString &command);
  QString GetODTViewerCLI() const;

  void SetPDFViewerCLI(const QString &command);
  QString GetPDFViewerCLI() const;

  void SetPSViewerCLI(const QString &command);
  QString GetPSViewerCLI() const;

  void SetDVIViewerCLI(const QString &command);
  QString GetDVIViewerCLI() const;

  void SetTextViewerCLI(const QString &command);
  QString GetTextViewerCLI() const;

  void SetBackupViewerCLI(const QString &command);
  QString GetBackupViewerCLI() const;

  /// User changed directories used for searching for reviews and papers
  bool GetPathsChanged() const { return(pathsChanged); }

  void SetCitationListFont(const QString &name, int size);
  QString GetCitationListFontName() const;
  int GetCitationListFontSize() const;

  void SetReviewFont(const QString &name, int size);
  QString GetReviewFontName() const;
  int GetReviewFontSize() const;

private slots:
  void AddPapersPath();

  void RemoveSelectedPapersPath();

  void SelectReadPapersPath();

private:

  /// User interface
  Ui::SettingsDialog *ui;

  /// Search paths were edited
  bool pathsChanged;
};

#endif  // SETTINGSDIALOG_H
