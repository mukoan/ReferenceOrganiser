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

#include "databasehandler.h"
#include "papermeta.h"
#include "reviewscanner.h"

#define VERSION "1.3"

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

  /// Scan the paperPaths to find new papers
  void ScanPaperPaths();

  /// Search dialog
  void Search();

  /// Name/rename database
  void DatabaseName();

  /// About dialog
  void About();

  /// Quit
  void Quit();

  /// Build simple list of papers with reviews  NOT USED
  void ScanRecords();

  /// Add a paper and some details
  void IngestPaper();

  /// Update view
  void UpdateView();

  /// Import review files
  void ImportReviews();

  /// Export the reviews as a single page HTML file
  void ExportHTML();

  /// Save the paper database as XML
  bool SaveDatabaseAs();

  /// Open new database dialog
  bool NewDatabase();

  /// Open load previous database dialog
  void LoadPreviousDatabase();

signals:
  /// New citation was made by generateKey()
  void haveNewCitation(const QString &key);

  /// Output after checking a citation
  void citationCheckResult(bool result);

protected:
   void closeEvent(QCloseEvent *);

private slots:
  /// Load last used database or start a new one if none available
  void startup();

  /// Set records
  void setRecords(const QVector<PaperMeta> &recs);

  /// Citations in database that are repeated
  void setDuplicates(const QStringList &duplicates);

  /// Show details about current database
  void showDatabaseDetails() const;

  /**
   * Set to current review
   * @param index   index into current view, either database or search results
   */
  void showDetailsForReview(int index);

  /// Display formatted text for review
  void displayFormattedDetails(const PaperMeta &meta_record);

  /// Review was selected, show review
  void selectItem();

  /// Select the given citation
  void selectCitation(const QString &cite);

  /// User clicked on link in review
  void gotoLinkedReview(const QUrl &link);

  /// Open the current paper
  void openCurrentPaper();

  /// Last accessed paper path
  void setLastPaperLocation(const QString &path);

  /// Show the review database status
  void showStatus();

  /// Rescan and update view
  void refresh();

  /// Rescan/refresh DEPRECATED
  void rescanPaths();

  /**
   * Change to a different view mode
   * @param mode  index to combo for view mode
   */
  void changeViewMode(int mode);

  /// Clear all tags in the filter
  void clearTagFilters();

  /// Change tag filter logic mode
  void toggleTagFilterLogic();

  /// Add the current tag selected in the combo to the filter list
  void addCurrentTagFilter();

  /// Edit an existing review
  void editReview();

  /// Create a new review
  void newReview();

  /// Create or modify a review
  void modifyOrNew(PaperMeta &meta);

  /// Delete a review
  void deleteReview();

  /// Move the paper file from current location to papers read dir
  bool movePaperToStorage(PaperMeta &meta);

  /// Generate a citation key, citation will be emitted
  void generateKey(const QString &authors, const QString &year);

  /// Check whether the citation is in use, emits citationCheckResult
  void checkCitationAvailable(const QString &citation);

  /// Save current database
  bool saveDatabase();

private:
  /// Load saved settings
  void loadSettings();

  /// Save settings
  void saveSettings();

  /// Parse database for all tags used; clears any current list
  void buildTagList();

  /// Find paper for given index
  void findPaper(int index);

  /// Clear review shown in GUI
  void clearDetails();

  // Checks if citation is in use
  bool checkCitationExists(const QString &text);

  Ui::OrganiserMain *ui;                 ///< User interface

  bool tagFilterAnd;                     ///< The tag filter is set to AND

  QStringList papersPaths;               ///< List of paths to papers
  QStringList newPapers;                 ///< List of new papers
  QString     readPapersPath;            ///< Where read papers should be ingested to

  QVector<PaperMeta> records;            ///< List of all records
  QVector<PaperMeta> searchResults;      ///< List of records obtained from searching
  QThread       *scanThread;
  ReviewScanner *scanner;

  QStringList duplicateRefs;             ///< List of references that have duplicates

  QStringList tags;                      ///< Tags used in database
  QString currentPaperPath;              ///< Full path, including extension
  QString currentReviewText;             ///< Including headers

  // Citation generation //

  int     prefMaximumCiteAuthors;        ///< Maximum number of names in a citation key
  int     prefMaximumCiteCharacters;     ///< Maximum number of characters in a citation key

  // Preference settings for viewers for different file formats //

  QString prefPDFViewer;                 ///< PDF viewer
  QString prefPSViewer;                  ///< PostScript viewer
  QString prefDOCViewer;                 ///< MS Word viewer
  QString prefTextViewer;                ///< Plain text viewer
  QString prefODTViewer;                 ///< LibreOffice viewer
  QString prefBackupViewer;              ///< Program to use if none of the above

  // Miscellaneous preferences and state //

  QString lastDatabaseFilename;          ///< Last database loaded
  QString lastExportedHTML;              ///< Location where HTML export saved file

  QDate   lastEnteredReview;             ///< When last review was added
  QString lastPaperPath;                 ///< Path to last paper
  DatabaseHandler db;                    ///< Handles loading and saving of the current database
};

#endif  // ORGANISERMAIN_H
