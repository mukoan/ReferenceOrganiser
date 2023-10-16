/**
 * @file   databasehandler.cpp
 * @brief  Handle loading and saving databases
 * @author Lyndon Hill
 * @date   2021.07.21
 */

#include <algorithm>

#include <QFile>
#include <QDebug>

#include "databasehandler.h"
#include "databasefilereader.h"
#include "databasefilewriter.h"

// Add a database file
bool DatabaseHandler::Load(const char *filename)
{
  DatabaseFileReader reader;

  QFile input(filename);
  if(!input.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    qDebug() << "Could not open file " << filename << " for reading.\n";
    return(false);
  }

  bool result = reader.Read(&input, databaseName, &database);
  if(result)
  {
    // Scan databse for metadata
    // Earliest and newest year of publication
    int r = 0;
    while(r < database.size())
    {
      QString year = database[r].year;
      if(!year.isEmpty())
      {
        bool ok;
        int yr;
        yr = year.toInt(&ok, 10);
        if(ok)
        {
          if(startYear == -1)
          {
            startYear = yr;
            endYear = yr;
          }
          else
          {
            if(yr < startYear) startYear = yr;
            if(yr > endYear) endYear = yr;
          }
        }
      }
      r++;
    }
  }

  return(result);
}

// Write database to file
bool DatabaseHandler::Save(const char *filename)
{
  DatabaseFileWriter writer;
  bool result = writer.Save(filename, databaseName, database);
  if(!result) qDebug() << "Handler: Failed to save database\n";

  return(result);
}

// New database
void DatabaseHandler::New(const QString &name)
{
  database.clear();
  databaseName = name;
}

// Sort database
void DatabaseHandler::Sort()
{
  std::sort(database.begin(), database.end());
}
