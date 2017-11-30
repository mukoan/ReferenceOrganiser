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

#include "paperrecord.h"

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
  void SetData(const QVector<PaperRecord> *recs) { records = recs; }

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

public slots:
  /// Begin search
  void process();

  /// Stop
  void halt();

signals:
  /// A result
  void result(const QString &cite);

  /// Search is finished
  void finished();

private:
  const QVector<PaperRecord> *records;

  QString keywords;

  /// Check title and review for keywords
  bool kwTitle, kwReview;

  ///
  QString searchAuthors;
  int yearStart, yearStop;

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
  void SetRecords(const QVector<PaperRecord> &recs)
  {
    citations = &recs;
  }

  /// Get results from search
  QStringList GetResults();

private slots:
  /// Perform search
  void search();

  /// Enable/disable author search UI element
  void setAuthorsSearchEnabled(bool tf);

  /// Enable/disable year range UI elements
  void setYearSearchEnabled(bool tf);

  /// Enable/disable keywords UI elements
  void setKeywordsSearchEnabled(bool tf);

  /// Add a result from the search in progress
  void addResult(const QString &cite);

  /// Search has finished
  void endSearch();

private:
  Ui::SearchDialog *ui;

  QThread  *searchThread;
  Searcher *searchObj;

  const QVector<PaperRecord> *citations;
  QStringList resultList;

  int numberResults;
};

#endif // SEARCHDIALOG_H
