/**
 * @file   searchdialog.cpp
 * @brief  Searching through citations/reviews
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#include <QFileDialog>
#include <QRegularExpression>

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
  keywords = keywords.replace(QRegularExpression(","), " ");

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

    keywords.remove(quote_start, quote_end-quote_start+1);
    keywords_list << quoted_string;
  }

  QStringList keywords_split = keywords.split(' ', Qt::SkipEmptyParts);
  keywords_list = keywords_list+keywords_split;

  // Basic: iterate through all records looking for matches

  QVector<PaperMeta>::const_iterator it = records->begin();

  while(it != records->end())
  {
    if(!doRun)
      break;

    // Direct match on paper path is enough
    if(!paperFile.isEmpty())
    {
      if(it->paperPath == paperFile)
      {
        emit result(it->citation, it->title);
      }
      ++it;
      continue;
    }  

    // Check year

    int year = it->year.toInt();

    if(yearStart != -1)
    {
      if((year < yearStart) || (year > yearStop))
      {
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

      QRegularExpression search_regexp(search_expression,
                                       QRegularExpression::CaseInsensitiveOption);

      for(int k = 0; k < keywords_list.size(); k++)
      {
        if(kwTitle)
        {
          if(it->title.contains(search_regexp))
          {
            keyword_match = true;
          }
        }

        if(kwReview)
        {
          if(it->review.contains(search_regexp))
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

    // Check match by author: only need one author to be a match

    if(!searchAuthors.isEmpty())
    {
      bool have_author = false;

      // separate authors into QStringList of individual authors
      QStringList author_list = searchAuthors.split(',', Qt::SkipEmptyParts);

      QString search_expression = "\\b(";
      for(int k = 0; k < author_list.size(); k++)
      {
        search_expression.append(author_list[k].toLower());
        if(k+1 < author_list.size())
          search_expression.append("|");
        else
          search_expression.append(")\\b");
      }

      QRegularExpression search_regexp(search_expression,
                                       QRegularExpression::CaseInsensitiveOption);

      for(int a = 0; a < author_list.size(); a++)
      {
        // if(it->authors.toLower().contains(search_regexp))
        if(it->authors.contains(search_regexp))
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

    // Must be a match if reach here
    emit result(it->citation, it->title);

    ++it;
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

  yearChange(0);
  numberResults = 0;

  connect(ui->paperSelectButton, &QToolButton::released,    this, &SearchDialog::selectPaperPath);
  connect(ui->yearStartSpin,     &QSpinBox::valueChanged,   this, &SearchDialog::yearChange);
  connect(ui->yearEndSpin,       &QSpinBox::valueChanged,   this, &SearchDialog::yearChange);
  connect(ui->searchButton,      &QPushButton::released,    this, &SearchDialog::search);
  connect(ui->closeButton,       &QPushButton::released,    this, &QDialog::accept);
  connect(ui->cancelButton,      &QPushButton::released,    this, &QDialog::reject);

  connect(ui->authorsCheck,      &QAbstractButton::toggled, this, &SearchDialog::searchTypeChanged);
  connect(ui->yearCheck,         &QAbstractButton::toggled, this, &SearchDialog::searchTypeChanged);
  connect(ui->keywordsCheck,     &QAbstractButton::toggled, this, &SearchDialog::searchTypeChanged);
  connect(ui->paperPathCheck,    &QAbstractButton::toggled, this, &SearchDialog::searchTypeChanged);
}

SearchDialog::~SearchDialog()
{
  delete ui;
}

// Set the range of papers available in the database
void SearchDialog::SetDatabaseYearRange(int first_year, int last_year)
{
  ui->yearStartSpin->setValue(first_year);
  ui->yearEndSpin->setValue(last_year);
}

// Get results from search
QStringList SearchDialog::GetResults()
{
  return(resultList);
}

// There was a change to the year range: enforce validity
void SearchDialog::yearChange(int)
{
  ui->yearStartSpin->setMaximum(ui->yearEndSpin->value());
  ui->yearEndSpin->setMinimum(ui->yearStartSpin->value());
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

  if(ui->paperPathCheck->isChecked())
  {
    searchObj->SetPaperPath(ui->paperPathEdit->text());
  }

  ui->searchButton->setText(tr("Halt"));

  searchObj->moveToThread(searchThread);
  connect(searchThread,     &QThread::started,      searchObj,    &Searcher::process);
  connect(searchObj,        &Searcher::result,      this,         &SearchDialog::addResult);
  connect(ui->searchButton, &QPushButton::released, searchObj,    &Searcher::halt);
  connect(searchObj,        &Searcher::finished,    this,         &SearchDialog::endSearch);
  connect(searchObj,        &Searcher::finished,    searchObj,    &Searcher::deleteLater);
  connect(searchThread,     &QThread::finished,     searchThread, &QThread::deleteLater);
  searchThread->start();

  QCoreApplication::processEvents();
}

// Open dialog to select a paper
void SearchDialog::selectPaperPath()
{
  QString paper_path = QFileDialog::getOpenFileName(this, tr("Select Paper"),
                                                          papersReadDir,
                                                          tr("Paper files (*.doc *.docx *.dvi *.odt *.ps *.ps.gz *.pdf *.txt)"));
  if(paper_path.isEmpty()) return;

  ui->paperPathEdit->setText(paper_path);
}

// Add a result from the search in progress
void SearchDialog::addResult(const QString &cite, const QString &title)
{
  QListWidgetItem *element = new QListWidgetItem(ui->citationResultsList);
  element->setText(cite);
  element->setToolTip(title);
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

// A search parameter enable was toggled
void SearchDialog::searchTypeChanged(bool)
{
  bool enable_search = ((ui->authorsCheck->isChecked()) || (ui->yearCheck->isChecked()) ||
                        (ui->keywordsCheck->isChecked()) || (ui->paperPathCheck->isChecked()));

  ui->searchButton->setEnabled(enable_search);
}
