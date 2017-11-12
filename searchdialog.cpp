/**
 * @file   searchdialog.cpp
 * @brief  Searching through citations/reviews
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#include <iostream>

#include "searchdialog.h"
#include "ui_searchdialog.h"
#include "reviewparser.h"

// Constructor
Searcher::Searcher() : QObject()
{
  yearStart = -1;
  yearStop  = -1;

  kwTitle   = false;
  kwReview  = false;

  records   = nullptr;
  doRun     = false;
}

// Begin search
void Searcher::process()
{
  if(!records)
  {
    emit finished();
    return;
  }

  doRun = true;

  // Make list of keywords to search for

  QStringList keywords_list;

  // convert commas to spaces
  keywords = keywords.replace(QRegExp(","), " ");

  int pos = 0;
  while(true)
  {
    int quote_start = keywords.indexOf("\"", pos);
    if(quote_start < 0) break;

    pos = quote_start+1;
    int quote_end = keywords.indexOf("\"", pos);
    if(quote_end < 0)
    {
      // Mismatched quotes
      break;
    }

    QString quoted_string = keywords.mid(quote_start+1, quote_end-(quote_start+1));

    // std::cerr << "Found quoted term '" << quoted_string.toStdString() << "'\n";
    keywords.remove(quote_start, quote_end-quote_start+1);
    keywords_list << quoted_string;
  }

  QStringList keywords_split = keywords.split(' ', QString::SkipEmptyParts);
  keywords_list = keywords_list+keywords_split;

  // Basic: iterate through all records looking for matches

  QVector<PaperRecord>::const_iterator it = records->begin();

  while(it != records->end())
  {
    // load review / parse year and authors

    QString     review_text;
    if(LoadReview(it->reviewPath, review_text))
    {
      int         year;
      QString     year_string;
      QString     authors;
      QString     title;

      bool is_match = true;

      ParseReview(it->reviewPath, review_text, authors, title, year_string);

      year = year_string.toInt();

      if(yearStart != -1)
      {
        if((year < yearStart) || (year > yearStop))
        {
          is_match = false;
          ++it;
          continue;
        }
      }

      // Match if keyword is in title or review

      if(kwTitle || kwReview)
      {
        bool keyword_match = false;

        // Make reg exp for matching keywords with word boundaries either side

        QString search_expression = "\\b(";
        for(int k = 0; k < keywords_list.size(); k++)
        {
          search_expression.append(keywords_list[k]);
          if(k+1 < keywords_list.size())
            search_expression.append("|");
          else
            search_expression.append(")\\b");
        }

        QRegExp search_regexp(search_expression, Qt::CaseInsensitive);

        for(int k = 0; k < keywords_list.size(); k++)
        {
          if(kwTitle)
          {
            if(title.contains(search_regexp))
            {
              keyword_match = true;
            }
          }

          if(kwReview)
          {
            if(review_text.contains(search_regexp))
            {
              keyword_match = true;
            }
          }
        }

        if(!keyword_match)
        {
          ++it;
          continue;
        }
      }

      // Only need one author to be a match
      if(!searchAuthors.isEmpty())
      {
        bool have_author = false;

        // separate authors into QStringList of individual authors
        QStringList author_list = searchAuthors.split(',', QString::SkipEmptyParts);

        QString search_expression = "\\b(";
        for(int k = 0; k < author_list.size(); k++)
        {
          search_expression.append(author_list[k].toLower());
          if(k+1 < author_list.size())
            search_expression.append("|");
          else
            search_expression.append(")\\b");
        }

        QRegExp search_regexp(search_expression, Qt::CaseInsensitive);

        for(int a = 0; a < author_list.size(); a++)
        {
          if(authors.toLower().contains(search_regexp))
          {
            have_author = true;
            break;
          }
        }

        if(!have_author)
        {
          ++it;
          continue;
        }
      }

      if(is_match)
      {
        // Citation is matched, add to results list
        emit result(it->citation);
      }
    }

    ++it;

    if(!doRun)
      break;
  }

  emit finished();
}

// Stop
void Searcher::halt()
{
  doRun = false;
}


SearchDialog::SearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchDialog)
{
  ui->setupUi(this);

  numberResults = 0;

  connect(ui->authorsCheck,  &QAbstractButton::toggled, this, &SearchDialog::setAuthorsSearchEnabled);
  connect(ui->yearCheck,     &QAbstractButton::toggled, this, &SearchDialog::setYearSearchEnabled);
  connect(ui->keywordsCheck, &QAbstractButton::toggled, this, &SearchDialog::setKeywordsSearchEnabled);

  connect(ui->searchButton,  &QPushButton::released,    this, &SearchDialog::search);
  connect(ui->closeButton,   &QPushButton::released,    this, &QDialog::accept);
  connect(ui->cancelButton,  &QPushButton::released,    this, &QDialog::reject);
}

SearchDialog::~SearchDialog()
{
  delete ui;
}

// Get results from search
QStringList SearchDialog::GetResults()
{
  return(resultList);
}

// Perform search
void SearchDialog::search()
{
  ui->citationResultsList->clear();
  resultList.clear();
  numberResults = 0;

  ui->busyWidget->start();

  // connect to set results
  searchThread = new QThread;
  searchThread->setObjectName("RefOrg-Search");

  searchObj = new Searcher;
  searchObj->SetData(citations);

  if(ui->authorsCheck->isChecked())
    searchObj->SetAuthors(ui->authorsEdit->text());
  else
    searchObj->SetAuthors("");

  if(ui->yearCheck->isChecked())
    searchObj->SetYears(ui->yearStartSpin->value(), ui->yearEndSpin->value());
  else
    searchObj->SetYears(-1, -1);

  if(ui->keywordsCheck->isChecked())
  {
    QString keywords_simple = ui->keywordsEdit->text().simplified().toLower();
    searchObj->SetKeywords(keywords_simple, ui->keywordsTitleCheck->isChecked(), ui->keywordsReviewCheck->isChecked());
  }
  else
    searchObj->SetKeywords("", false, false);

  ui->searchButton->setText(tr("Halt"));

  searchObj->moveToThread(searchThread);
  connect(searchThread,     SIGNAL(started()),             searchObj,    SLOT(process()));
  connect(searchObj,        SIGNAL(result(const QString)), this,         SLOT(addResult(const QString)));
  connect(ui->searchButton, SIGNAL(clicked()),             searchObj,    SLOT(halt()));
  connect(searchObj,        SIGNAL(finished()),            this,         SLOT(endSearch()));
  connect(searchObj,        SIGNAL(finished()),            searchObj,    SLOT(deleteLater()));
  connect(searchThread,     SIGNAL(finished()),            searchThread, SLOT(deleteLater()));
  searchThread->start();

  QCoreApplication::processEvents();
}

// Enable/disable author search UI element
void SearchDialog::setAuthorsSearchEnabled(bool tf)
{
  ui->authorsEdit->setEnabled(tf);
}

// Enable/disable year range UI elements
void SearchDialog::setYearSearchEnabled(bool tf)
{
  ui->yearStartSpin->setEnabled(tf);
  ui->yearEndSpin->setEnabled(tf);
}

// Enable/disable keywords UI elements
void SearchDialog::setKeywordsSearchEnabled(bool tf)
{
  ui->keywordsEdit->setEnabled(tf);
  ui->keywordsTitleCheck->setEnabled(tf);
  ui->keywordsReviewCheck->setEnabled(tf);
}

// Add a result from the search in progress
void SearchDialog::addResult(const QString &cite)
{
  QListWidgetItem *element = new QListWidgetItem(ui->citationResultsList);
  element->setText(cite);
  numberResults++;

  resultList << cite;
}

// Search has finished
void SearchDialog::endSearch()
{
  // stop busywidget
  ui->busyWidget->stop();
  ui->resultsStatusLabel->setText(tr("%1 results found").arg(numberResults));

  ui->searchButton->setText(tr("Search"));
}
