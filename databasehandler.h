/**
 * @file   databasehandler.h
 * @brief  Handle loading and saving databases
 * @author Lyndon Hill
 * @date   2021.07.21
 */

#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include <QVector>
#include <QString>

#include "papermeta.h"

class DatabaseHandler
{
public:
  DatabaseHandler() : databaseName("Default database"), startYear(-1), endYear(-1) { }

  /**
   * Add a database file
   * @param filename  Name of database file
   * @return true if successfully opened
   */
  bool Load(const char *filename);

  /// Write database to file
  bool Save(const char *filename);

  /// New database
  void New(const QString &name);

  /// Sort the database by citation key
  void Sort();

  QVector<PaperMeta> database;
  QString            databaseName;
  int                startYear;
  int                endYear;
};

#endif  // DATABASEHANDLER_H
