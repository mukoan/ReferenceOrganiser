/**
 * @file   reviewparser.cpp
 * @brief  Load and parse a review
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#include <stdio.h>
#include <iostream>
#include <QStringList>

#include "reviewparser.h"


// Load a review
bool LoadReview(const QString &filename, QString &review)
{
  FILE *fp;
  if((fp = fopen(filename.toUtf8().constData(), "rb")))
  {
    fseek(fp, 0, SEEK_END);
    size_t data_size = ftell(fp);
    rewind(fp);

    char *data = (char *)malloc(data_size+1);
    if(!data)
    {
      fclose(fp);
      return(false);
    }

    fread(data, data_size, 1, fp);
    fclose(fp);

    data[data_size] = 0;

    review = data;

    free(data);

    return(true);
  }

  return(false);
}

// Get embedded details from the review
void ParseReview(const QString &filename, const QString &review, QString &authors, QString &title, QString &year)
{
  ParseYearFromCitation(filename, year);

  // Title on first line

  title.clear();

  QStringList lines = review.split('\n');
  if(lines.size() > 0)
  {
    // std::cerr << "Title = '" << lines[0].toStdString() << "'\n";
    title = lines[0];
  }

  // Second line either filename in curly braces or comma separated authors list

  authors.clear();
  if(lines.size() > 1)
  {
    QString potential_authors(lines[1].simplified());

    if((potential_authors[0] == '{') && (potential_authors[potential_authors.size()-1] == '}'))
    {
      // line is reference - this is from an deprecated version of the review formatting
    }
    else
      authors = potential_authors;
  }
}

// Find the year appended at the end of the string
bool ParseYearFromCitation(const QString &cite, QString &year)
{
  int year_num;

  // Get year from filename; find digits from end of string

  int  year_end     = 0;
  int  year_start   = 0;
  bool number_start = false;

  QString::const_reverse_iterator rit = cite.rbegin();
  while(rit != cite.rend())
  {
    if(number_start && !rit->isDigit())   // reached end of number
      break;

    if(!number_start && rit->isDigit())
    {
      number_start = true;
    }

    if(!number_start) year_end++;
     year_start++;
    ++rit;
  }

  year_start--;

  if(!number_start || ((year_start >= cite.size()) && (year_end >= cite.size())))
  {
    year_num = -1;
    year.clear();
    return(false);
  }
  else
  {
    // Convert year_start and year_end to string indices

    year_start = cite.size()-year_start-1;
    year_end   = cite.size()-year_end-1;

    // Get year portion of string

    QString year_string = cite.mid(year_start, year_end-year_start+1);
    year_num = year_string.toInt();

    // Handle 2 digit years

    if(year_num < 50)
      year_num += 2000;
    else if((year_num > 50) && (year_num < 100))
      year_num += 1900;

    year = QString::number(year_num);
  }

  return(true);
}

// Trim title and authors line from review
void TrimReviewHeader(QString &review)
{
  if(review.isEmpty()) return;

  QStringList lines = review.split('\n');
  if(lines.count() < 2) return;

  QStringList::iterator it = lines.begin();

  // Erase first two lines if they are not empty; erase following blank line
  if(!it->isEmpty()) it = lines.erase(it);
  if(it == lines.end()) return;
  if(!it->isEmpty()) it = lines.erase(it);
  if(it == lines.end()) return;
  if(it->isEmpty())  it = lines.erase(it);

  review.clear();
  while(it != lines.end())
  {
    review = review + *it + "\n";
    ++it;
  }
}

// Parse authors from list
QStringList ParseAuthors(const QString &authors_csv)
{
  return(authors_csv.split(",", QString::SkipEmptyParts));
}
