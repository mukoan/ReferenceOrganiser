/**
 * @file   databasefilereader.h
 * @brief  Reference Organiser Database XML reader
 * @author Lyndon Hill
 * @date   2019.04.23
 */

#ifndef DATABASEFILEREADER_H
#define DATABASEFILEREADER_H

#include <QXmlStreamReader>
#include <QString>
#include <QVector>

#include "papermeta.h"

/**
 * @brief Read and parse a database file
 */
class DatabaseFileReader
{
public:
  DatabaseFileReader();

  /// Destructor
  ~DatabaseFileReader();

  /**
   * Read the database
   * @param device   where the database is being read from
   * @param database  the database being read to
   */
  bool Read(QIODevice *device, QString &name, QVector<PaperMeta> *database);

private:
  /// Read the list of records
  void readRecordList();

  /// Read the basic record info
  void readRecordDetails();

  /// Read meta data describing the paper
  void readPaperMeta();

  /// Read meta data describing the readers opinion
  void readReaderMeta();

  /// Read meta data describing the reviewers opinion
  void readReviewerMeta();

  bool        databaseStarted;        ///< Reading has started
  int         databaseFileVersion;    ///< Version of the database file
  PaperMeta   record;                 ///< Current record/review being read

  /// Copy of the pointer to the database being read
  QVector<PaperMeta> *targetDatabase;

  /// The low level reader
  QXmlStreamReader reader;
};

#endif  // DATABASEFILEREADER_H
