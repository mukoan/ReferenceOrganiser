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

#define VERSION "1.1"

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

  /// Citations in database that are repeated
  void setDuplicates(const QStringList &duplicates);

  /**
   * Set to current review
   * @param fname   citation, no path
   */
  void showDetailsForReview(const QString fname);
  void showDetailsForReview(int index);

  /**
   * Display formatted text for review
   * @param filename     filename for the review
   * @param review_text  text of review to be displayed
   */
  void displayFormattedDetails(const QString &filename, const QString &review_text);

  /// Review was selected, show review
  void selectItem();

  /// Select the given citation
  void selectCitation(const QString &cite);

  /// User clicked on link in review
  void gotoLinkedReview(const QUrl &link);

  /// Open the current paper
  void openCurrentPaper();

  /// Show the review database status
  void showStatus();

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

  /**
   * Update after a review was saved
   * @param refresh    update review list
   */
  void saveDone(bool refresh);

private:
  /// Load saved settings
  void loadSettings();

  /// Save settings
  void saveSettings();

  /// Find paper for given file reference, filename does not include extension
  void findPaper(const QString &filename);

  // Find paper for given index
  void findPaper(int index);

  /// Clear review shown in GUI
  void clearDetails();

  Ui::OrganiserMain *ui;                 ///< User interface

  QStringList reviewsPaths;              ///< List of paths to reviews
  QStringList papersPaths;               ///< List of paths to papers

  QVector<PaperRecord> records;          ///< List of all records
  QVector<PaperRecord> searchResults;    ///< List of records obtained from searching
  QThread       *scanThread;
  ReviewScanner *scanner;

  QStringList duplicateRefs;             ///< List of references that have duplicates

  /// Full path, including extension
  QString currentPaperPath;

  /// Including headers
  QString currentReviewText;

  // Citation generation //

  int     prefMaximumCiteAuthors;        ///< Maximum number of names in a citation key
  int     prefMaximumCiteCharacters;     ///< Maximum number of characters in a citation key

  // Preference settings for viewers for different file formats //

  QString prefPDFViewer;                 ///< PDF viewer
  QString prefPSViewer;                  ///< PostScript viewer
  QString prefDOCViewer;                 ///< MS Word viewer
  QString prefTextViewer;                ///< Plain text viewer
  QString prefBackupViewer;              ///< Program to use if none of the above

  // Miscellaneous preferences and state //

  int     prefReviewOutputPath;          ///< Preferred path in reviewsPaths to store new reviews
  QString lastExportedHTML;              ///< Location where HTML export saved file
};

#endif // ORGANISERMAIN_H
