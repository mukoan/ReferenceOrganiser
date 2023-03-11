/**
 * @file   databasefilewriter.h
 * @brief  Reference Organiser Database XML writer
 * @author Lyndon Hill
 * @date   2021.01.09
 */

#ifndef DATABASEFILEWRITER_H
#define DATABASEFILEWRITER_H

#include <QString>
#include <QVector>

#include "papermeta.h"

#define DATABASE_VERSION "1"

/// Write a database file
class DatabaseFileWriter
{
public:
  /// Constructor
  DatabaseFileWriter();

  /**
   * Save a database to a file
   * @param filename   full path and name to save to; usually with .rodb extension
   * @param name       the name of the database
   * @param records    the database to save
   */
  bool Save(const QString &filename, const QString &name, const QVector<PaperMeta> &records);

  /// Version number, as a string
  QString version;
};

#endif  // DATABASEFILEWRITER_H
