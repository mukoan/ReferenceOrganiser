/**
 * @file   searchdialog.h
 * @brief  Searching through citations/reviews
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QVector>
#include <QStringList>
#include <QThread>

#include "papermeta.h"

/**
 * @brief Object to perform search in another thread
 */
class Searcher : public QObject
{
Q_OBJECT

public:
  /// Constructor
  Searcher();

  /// Set the data to search
  void SetData(const QVector<PaperMeta> *recs) { records = recs; }

  /// Set search terms
  void SetKeywords(const QString &words, bool title, bool review)
  {
    keywords = words;
    kwTitle  = title;
    kwReview = review;
  }

  /// Authors to search for, CSV format
  void SetAuthors(const QString &terms) { searchAuthors = terms; }

  /// Set range (in years) of papers to search
  void SetYears(int start, int end) { yearStart = start; yearStop = end; }

  /// Path to paper saught
  void SetPaperPath(const QString &path) { paperFile = path; }

public slots:
  /// Begin search
  void process();

  /// Stop
  void halt();

signals:
  /// A result
  void result(const QString &cite, const QString &title);

  /// Search is finished
  void finished();

private:
  /// A pointer to the main database
  const QVector<PaperMeta> *records;

  /// Keywords that are being searched for
  QString keywords;

  /// Check title and review for keywords
  bool kwTitle, kwReview;

  /// Search parameters
  QString searchAuthors;
  int yearStart, yearStop;

  QString paperFile;

  /// Keep the search running
  bool doRun;
};


namespace Ui {
class SearchDialog;
}

/**
 * @brief Dialog to handle search queries
 */
class SearchDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SearchDialog(QWidget *parent = 0);
  ~SearchDialog();

  /// Set the records that can be searched
  void SetRecords(const QVector<PaperMeta> &recs)
  {
    citations = &recs;
  }

  /// Get results from search
  QStringList GetResults();

  /// Where read papers get ingested/stored to, for search by paper path
  void SetPapersRead(const QString &loc) { papersReadDir = loc; }

  /// Set the range of papers available in the database
  void SetDatabaseYearRange(int first_year, int last_year);

private slots:
  /// There was a change to the year range: enforce validity
  void yearChange(int);

  /// Perform search
  void search();

  /// Open dialog to select a paper
  void selectPaperPath();

  /// Add a result from the search in progress
  void addResult(const QString &cite, const QString &title);

  /// Search has finished
  void endSearch();

  /// A search parameter enable was toggled
  void searchTypeChanged(bool);

private:
  Ui::SearchDialog *ui;

  QThread  *searchThread;
  Searcher *searchObj;

  const QVector<PaperMeta> *citations;
  QStringList resultList;

  QString papersReadDir;

  int numberResults;
};

#endif  // SEARCHDIALOG_H
