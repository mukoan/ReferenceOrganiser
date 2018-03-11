/**
 * @file   reviewscanner.cpp
 * @brief  Search object to find reviews and papers
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#include <iostream>
#include <algorithm>

#include <QDir>
#include <QFileInfoList>
#include <QVector>

#include "reviewscanner.h"

ReviewScanner::ReviewScanner(QObject *parent) : QObject(parent)
{
  doRun = false;
}

// Set paths to look for files
void ReviewScanner::SetPaths(const QStringList &reviews, const QStringList &papers)
{
  rpaths = reviews;
  ppaths = papers;
}

void ReviewScanner::process()
{
  doRun = true;

  QVector<PaperRecord> new_records;
  QStringList duplicate_reviews;

  // Scan reviews
  for(int p = 0; p < rpaths.size(); p++)
  {
    QDir path(rpaths[p]);

    QFlags<QDir::Filter>  myspec  = QDir::Files | QDir::NoDotAndDotDot;
    QFlags<QDir::SortFlag> mysort = QDir::IgnoreCase | QDir::Name;

    QFileInfoList listing;
    listing = path.entryInfoList(myspec, mysort);

    if(listing.size())
    {
      QList<QFileInfo>::Iterator fi;
      fi = listing.begin();

      while(fi != listing.end())
      {
        QString fname = fi->fileName();

        // If file has extension as paper then ignore
        QString extension = fname.section('.', -1);
        extension = extension.toLower();
        if((extension == "pdf") || (extension == "ps") || (extension == "doc"))
        {
          ++fi;
          continue;
        }

        if(fname.section('.', -2, -1).toLower() == "ps.gz")
        {
          ++fi;
          continue;
        }

        PaperRecord pr;
        pr.citation   = fname;
        pr.reviewPath = rpaths[p] + QDir::separator() + fname;
        new_records.push_back(pr);

        ++fi;
      }
    }
  }

  // Sort by citation so that matching papers to reviews is faster
  std::sort(new_records.begin(), new_records.end());

  // Scan through all available papers, find in list so far or generate new record

  QVector<QFileInfoList> part_listings;
  QVector<QList<QFileInfo>::Iterator> part_iterators;

  QStringList all_paper_dirs = ppaths;

  for(int a = 0; a < all_paper_dirs.size(); a++)
  {
    QDir path(all_paper_dirs[a]);

    QFlags<QDir::Filter>  myspec  = QDir::AllEntries | QDir::NoDotAndDotDot;
    QFlags<QDir::SortFlag> mysort = QDir::IgnoreCase | QDir::Name;

    QFileInfoList listing;
    listing = path.entryInfoList(myspec, mysort);

    if(listing.size())
    {
      QList<QFileInfo>::Iterator fi;
      fi = listing.begin();

      QString basename;

      while(fi != listing.end())
      {
        if(fi->isDir())
        {
          all_paper_dirs.push_back(fi->absoluteFilePath());
          ++fi;
          continue;
        }

        // Check file is a paper
        QString extension = fi->fileName().section('.', -1).toLower();

        if(extension == "gz")
        {
          if(fi->fileName().section('.', -2, -1).toLower() != "ps.gz")
          {
            ++fi;
            continue;
          }
          else
            basename = fi->fileName().section('.', 0, -3);
        }
        else if((extension != "pdf") && (extension != "ps") && (extension != "doc"))
        {
          ++fi;
          continue;
        }
        else
          basename = fi->fileName().section('.', 0, -2);

        // Check if paper is already in records

        bool in_records = false;

        for(int r = 0; r < new_records.size(); r++)
        {
          if(new_records[r].citation == basename)
          {
            // Consider if another paper with same filename exists...
            if(new_records[r].paperPath.isEmpty())
            {
              in_records = true;
              new_records[r].paperPath = all_paper_dirs[a] + QDir::separator() + fi->fileName();
              break;
            }
          }
          else if(new_records[r].citation > basename)
          {
            // No point in looking further as reviews were sorted prior
            break;
          }
        }

        if(!in_records)
        {
          PaperRecord pr;
          pr.citation  = basename;
          pr.paperPath = all_paper_dirs[a] + QDir::separator() + fi->fileName();
          new_records.push_back(pr);
        }

        ++fi;
      }
    }
  }

  // Sort by citation or by review or paper if only they exist
  std::sort(new_records.begin(), new_records.end());

  // Find duplicates: records are sorted so they should be next to each other
  for(int r = 1; r < new_records.size(); r++)
  {
    if(new_records[r].citation == new_records[r-1].citation)
    {
      duplicate_reviews.push_back(new_records[r].citation);
    }
  }

  emit foundDuplicates(duplicate_reviews);
  emit results(new_records);
  emit finished();
}

// Stop
void ReviewScanner::halt()
{
  doRun = false;
}
