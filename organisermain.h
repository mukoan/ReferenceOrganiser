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
#include <QRegularExpression>
#include <QThread>

#include "databasehandler.h"
#include "papermeta.h"
#include "reviewscanner.h"
#include "history.h"

#define VERSION "1.3"

/// Maximum number of history items to show in the menu
#define MAX_HISTORY_ENTRIES 15

// Returns text with accents removed
QString RemoveAccents(const QString &text);


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

  /// Potential duplicates to given query on checkDuplicates()
  void duplicatesFound(const QVector<PaperMeta> &dupes);

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

  /// Rebuild history menu
  void updateHistoryMenu();

  /// Go to the record from the history
  void goHistoricAction();

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

  /// The filter tag combo was changed - add to the tag list
  void selectCurrentTag(int);

  /// Edit an existing review
  void editReview();

  /// Create a new review
  void newReview();

  /// Create or modify a review
  void modifyOrNew(PaperMeta &meta);

  /// Delete a review
  void deleteReview();

  /// Enable tag filtering
  void setTagFilteringEnabled(bool enable);

  /// Move the paper file from current location to papers read dir
  bool movePaperToStorage(PaperMeta &meta);

  /// Generate a citation key, citation will be emitted
  void generateKey(const QString &authors, const QString &year);

  /// Check whether the citation is in use, emits citationCheckResult
  void checkCitationAvailable(const QString &citation);

  /// Search database for papers similar to that given
  void searchDuplicates(const QString &authors, const QString &title, const QString &year);

  /// Save current database - always tries to save
  bool saveDatabase();

  /// Save database periodically
  void periodicSave();

private:
  /// Load saved settings
  void loadSettings();

  /// Save settings
  void saveSettings();

  /// Parse database for all tags used; clears any current list
  void buildTagList();

  /// Find paper for given index
  void findPaper(int index);

  /// Get bibtext for record
  QString getBibtex(const PaperMeta &meta_record) const;

  /// Clear review shown in GUI
  void clearDetails();

  /// Checks if citation is in use
  bool checkCitationExists(const QString &text);

  /// Utility function: abbreviates string
  static QString shortenString(const QString &src, int max_length = 50);

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

  History userHistory;                   ///< What user has been doing

  // Citation generation //

  int     prefMaximumCiteAuthors;        ///< Maximum number of names in a citation key
  int     prefMaximumCiteCharacters;     ///< Maximum number of characters in a citation key

  // Preference settings for viewers for different file formats //

  QString prefPDFViewer;                 ///< PDF viewer
  QString prefPSViewer;                  ///< PostScript viewer
  QString prefDOCViewer;                 ///< MS Word viewer
  QString prefTextViewer;                ///< Plain text viewer
  QString prefODTViewer;                 ///< LibreOffice viewer
  QString prefDVIViewer;                 ///< DVI viewer
  QString prefBackupViewer;              ///< Program to use if none of the above

  // Miscellaneous preferences and state //

  QString lastDatabaseFilename;          ///< Last database loaded
  QString lastExportedHTML;              ///< Location where HTML export saved file

  QDate   lastEnteredReview;             ///< When last review was added
  QString lastPaperPath;                 ///< Path to last paper
  DatabaseHandler db;                    ///< Handles loading and saving of the current database

  QString prefReviewEditFontName;        ///< Family of font used for editing reviews
  int     prefReviewEditFontSize;        ///< Size of font used for editing reviews

  QRegularExpression matchBracesExpression;  ///< Match braces for converting review link markdown
  QRegularExpression hyperlinkRegExpression; ///< Match hyperlink
};

#endif  // ORGANISERMAIN_H
