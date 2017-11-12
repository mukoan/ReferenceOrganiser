/**
 * @file   organisermain.h
 * @brief  Main window
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#ifndef ORGANISERMAIN_H
#define ORGANISERMAIN_H

#include <QMainWindow>
#include <QStringList>
#include <QListWidgetItem>
#include <QVector>
#include <QThread>

#include "paperrecord.h"
#include "reviewscanner.h"

#define VERSION "1.0"

namespace Ui {
class OrganiserMain;
}


/// Main window operations
class OrganiserMain : public QMainWindow
{
  Q_OBJECT

public:
  /// Constructor
  explicit OrganiserMain(QWidget *parent = 0);

  /// Destructor
  ~OrganiserMain();

public slots:
  /// Handle the settings dialog
  void Settings();

  /// Scan all the reviews and populate the refList
  void ScanPaths();

  /// Search dialog
  void Search();

  /// About dialog
  void About();

  /// Quit
  void Quit();

  /// Build simple list of papers with reviews
  void ScanRecords();

  /// Update view
  void UpdateView();

  /// Import files; e.g. review files, BibTex, etc
  // void Import();

  /// Export the reviews as a single page HTML file
  void ExportHTML();

private slots:
  /// Set records
  void setRecords(const QVector<PaperRecord> &recs);

  /**
   * Set to current review
   * @param fname   citation, no path
   */
  void showDetailsForReview(const QString fname);

  /**
   * Display formatted text for review
   * @param filename     filename for the review
   * @param review_text  text of review to be displayed
   */
  void displayFormattedDetails(const QString &filename, const QString &review_text);

  /**
   * Select a review
   * @param current   review that was selected in the main window
   * @param previous  last review that was selected (unused)
   */
  void selectItem(QListWidgetItem *current, QListWidgetItem *previous);

  /// User clicked on link in review
  void gotoLinkedReview(const QUrl &link);

  /// Open the current paper
  void openCurrentPaper();

  /// Rescan/refresh
  void rescanPaths();

  /// (Re)Scan records and update view
  void refreshView();

  /**
   * Change to a different view mode
   * @param mode  index to combo for view mode
   */
  void changeViewMode(int mode);

  /// Edit an existing review
  void editReview();

  /// Create a new review
  void newReview();

  /// Delete a review
  void deleteReview();

  /// Update after a review was saved
  void saveDone();

private:
  /// Load saved settings
  void loadSettings();

  /// Save settings
  void saveSettings();

  /// Find paper for given file reference, filename does not include extension
  void findPaper(const QString &filename);

  /// Clear review shown in GUI
  void clearDetails();

  Ui::OrganiserMain *ui;

  QStringList reviewsPaths;            ///< List of paths to reviews
  QStringList papersPaths;             ///< List of paths to papers

  QVector<PaperRecord> records;        ///< List of all records
  QVector<PaperRecord> searchResults;  ///< List of records obtained from searching
  QThread       *scanThread;
  ReviewScanner *scanner;

  int prefReviewOutputPath;            ///< Preferred path in reviewsPaths to store new reviews

  /// Full path, including extension
  QString currentPaperPath;

  /// Including headers
  QString currentReviewText;

  // Citation generation
  int prefMaximumCiteAuthors;
  int prefMaximumCiteCharacters;

  // Viewers
  QString prefPDFViewer;
  QString prefPSViewer;
  QString prefDOCViewer;
  QString prefTextViewer;
  QString prefBackupViewer;

  QString lastExportedHTML;            ///< Location where HTML export saved file
};

#endif // ORGANISERMAIN_H
