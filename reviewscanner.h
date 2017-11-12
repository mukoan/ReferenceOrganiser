/**
 * @file   reviewscanner.h
 * @brief  Search object to find reviews and papers
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#ifndef REVIEWSCANNER_H
#define REVIEWSCANNER_H

#include <QObject>
#include <QStringList>
#include <QVector>

#include "paperrecord.h"

/**
 * @brief Scan paths and find all papers and reviews
 */
class ReviewScanner : public QObject
{
  Q_OBJECT

public:
  explicit ReviewScanner(QObject *parent = 0);

  /// Set paths to look for files
  void SetPaths(const QStringList &reviews, const QStringList &papers);

signals:
  /// Results
  void results(const QVector<PaperRecord> &records);

  /// Scan is finished
  void finished();

public slots:
  /// Begin search
  void process();

  /// Stop
  void halt();

private:
  bool doRun;

  QStringList rpaths, ppaths;
};

#endif // REVIEWSCANNER_H
