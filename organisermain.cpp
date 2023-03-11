/**
 * @file   organisermain.cpp
 * @brief  Main window
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#include "organisermain.h"
#include "ui_organisermain.h"
#include "createdatabasedialog.h"
#include "settingsdialog.h"
#include "databasenamedialog.h"
#include "metadialog.h"
#include "searchdialog.h"
#include "reviewparser.h"
#include "recordlistitem.h"

#include <iostream>
#include <fstream>
#include <stdio.h>

#include <QSettings>
#include <QDir>
#include <QFileInfoList>
#include <QList>
#include <QListWidgetItem>
#include <QChar>
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QFile>
#include <QDate>
#include <QMessageBox>
#include <QDebug>

OrganiserMain::OrganiserMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OrganiserMain)
{
  ui->setupUi(this);

  qRegisterMetaType<QVector<PaperMeta>>("QVector<PaperMeta>");

  QIcon myicon;
  QPixmap pix1x(":/Assets/reforg.png");
  QPixmap pix2x(":/Assets/reforg@2x.png");
  pix2x.setDevicePixelRatio(2);

  myicon.addPixmap(pix1x);
  myicon.addPixmap(pix2x);
  setWindowIcon(myicon);

  scanThread = nullptr;

  loadSettings();

  connect(ui->actionImport_Reviews,  &QAction::triggered,                this, &OrganiserMain::ImportReviews);
  connect(ui->actionExport_HTML,     &QAction::triggered,                this, &OrganiserMain::ExportHTML);
  connect(ui->actionRefreshView,     &QAction::triggered,                this, &OrganiserMain::refresh);
  connect(ui->actionAbout,           &QAction::triggered,                this, &OrganiserMain::About);
  connect(ui->actionQuit,            &QAction::triggered,                this, &OrganiserMain::Quit);

  connect(ui->actionCurrentDetails,  &QAction::triggered,                this, &OrganiserMain::showDatabaseDetails);
  connect(ui->actionName,            &QAction::triggered,                this, &OrganiserMain::DatabaseName);
  connect(ui->actionNewDatabase,     &QAction::triggered,                this, &OrganiserMain::NewDatabase);
  connect(ui->actionLoad_Previous_Database, &QAction::triggered,         this, &OrganiserMain::LoadPreviousDatabase);
  connect(ui->actionSave_As,         &QAction::triggered,                this, &OrganiserMain::SaveDatabaseAs);
  connect(ui->actionPreferences,     &QAction::triggered,                this, &OrganiserMain::Settings);
  connect(ui->actionStatus,          &QAction::triggered,                this, &OrganiserMain::showStatus);

  ui->actionAbout->setMenuRole(QAction::AboutRole);
  ui->actionPreferences->setMenuRole(QAction::PreferencesRole);
  ui->actionQuit->setMenuRole(QAction::QuitRole);

  connect(ui->openPaperButton,       &QPushButton::released,             this, &OrganiserMain::openCurrentPaper);
  connect(ui->newReviewButton,       &QPushButton::released,             this, &OrganiserMain::newReview);
  connect(ui->editButton,            &QPushButton::released,             this, &OrganiserMain::editReview);
  connect(ui->deleteButton,          &QPushButton::released,             this, &OrganiserMain::deleteReview); // Delete complete reference
  connect(ui->searchButton,          &QPushButton::released,             this, &OrganiserMain::Search);

  connect(ui->refList,               &QListWidget::itemSelectionChanged, this, &OrganiserMain::selectItem);

  connect(ui->viewCombo,             QOverload<int>::of(&QComboBox::currentIndexChanged),
                                                                         this, &OrganiserMain::changeViewMode);
  connect(ui->tagFilterClearButton,  &QToolButton::released,             this, &OrganiserMain::clearTagFilters);
  connect(ui->tagFilterAddButton,    &QToolButton::released,             this, &OrganiserMain::addCurrentTagFilter);
  connect(ui->tagFilterEdit,         &QLineEdit::textChanged,            this, &OrganiserMain::UpdateView);
  connect(ui->tagFilterLogicButton,  &QToolButton::released,             this, &OrganiserMain::toggleTagFilterLogic);
  connect(ui->detailsViewer,         &QTextBrowser::anchorClicked,       this, &OrganiserMain::gotoLinkedReview);

  ui->tagFilterCombo->setEnabled(false);
  tagFilterAnd = false;

  QTimer::singleShot(0, this, &OrganiserMain::startup);
}

OrganiserMain::~OrganiserMain()
{
  delete ui;
}

// Build simple list of papers with reviews - to be made part of import function  UNUSED
void OrganiserMain::ScanRecords()
{
/*
  ui->busyWidget->start();
  records.clear();

  scanThread = new QThread;
  scanThread->setObjectName("RefOrg-Scan");

  scanner = new ReviewScanner;
  scanner->SetPaths(reviewsPaths, papersPaths);

  scanner->moveToThread(scanThread);
  connect(scanThread, SIGNAL(started()), scanner,                          SLOT(process()));
  connect(scanner,    SIGNAL(results(const QVector<PaperMeta> &)), this,   SLOT(setRecords(const QVector<PaperMeta> &)));
  connect(scanner,    SIGNAL(foundDuplicates(const QStringList &)),  this, SLOT(setDuplicates(const QStringList &)));
  // connect(ui->scanButton, SIGNAL(clicked()), scanner, SLOT(halt()));
  connect(scanner,    SIGNAL(finished()), scanner,                         SLOT(deleteLater()));
  connect(scanThread, SIGNAL(finished()), scanThread,                      SLOT(deleteLater()));
  scanThread->start();
*/
}

// Update view - use viewCombo and records to update view shown
void OrganiserMain::UpdateView()
{
  // Clear all

  ui->editButton->setEnabled(false);
  ui->deleteButton->setEnabled(false);
  ui->openPaperButton->setEnabled(false);
  ui->refList->clear();
  ui->detailsViewer->clear();
  ui->paperPathLabel->clear();

  int selected_records = 0;

  if(ui->viewCombo->currentIndex() == 4)
  {
    // Show search results

    for(int r = 0; r < searchResults.size(); r++)
    {
      RecordListItem *element = new RecordListItem(ui->refList, r);
      element->setText(searchResults[r].citation);
      selected_records++;
    }
  }
  else
  {
    if(ui->viewCombo->currentIndex() == 1)
    {
      for(int r = 0; r < newPapers.size(); r++)
      {
        RecordListItem *element = new RecordListItem(ui->refList, r);
        element->setText(newPapers[r].section("/", -1));
        selected_records++;
      }
    }
    else
    {
      QStringList tags_of_interest = ui->tagFilterEdit->text().split(",", Qt::SkipEmptyParts);

      for(int r = 0; r < db.database.size(); r++)
      {
        // Apply tag filter

        QStringList tags_of_record = db.database[r].tags.split(",", Qt::SkipEmptyParts);
        bool filter_reject = false;
        int matching_tags = 0;

        for(int t = 0; t < tags_of_interest.size(); t++)
        {
          if(!tags_of_record.contains(tags_of_interest[t]))
          {
            if(tagFilterAnd)
            {
              filter_reject = true;
              break;
            }
          }
          else
          {
            matching_tags++;
          }
        }

        if(!tagFilterAnd)
        {
          if((matching_tags == 0) && (tags_of_interest.size() > 0))
            filter_reject = true;
        }

        if(filter_reject) continue;

        // Show info based on view mode

        switch(ui->viewCombo->currentIndex())
        {
          case 0: // reviews (entries in database)
          {
            RecordListItem *element = new RecordListItem(ui->refList, r);
            element->setText(db.database[r].citation);
            element->setToolTip(db.database[r].title);
            selected_records++;
          }
          break;

          case 1: // new papers - do nothing here
          break;

          case 2: // reviewed papers
          if(!db.database[r].review.isEmpty())
          {
            RecordListItem *element = new RecordListItem(ui->refList, r);
            element->setText(db.database[r].citation);
            element->setToolTip(db.database[r].title);
            selected_records++;
          }
          break;

          case 3: // incomplete reviews
          if(!db.database[r].reader.finished)
          {
            RecordListItem *element = new RecordListItem(ui->refList, r);
            element->setText(db.database[r].citation);
            element->setToolTip(db.database[r].title);
            selected_records++;
          }
          break;
        }
      }
    }
  }

  ui->numberPapersLabel->setText(QString("%1").arg(selected_records));

  showDatabaseDetails();
}

// About dialog
void OrganiserMain::About()
{
  QMessageBox::about(this, "About Reference Organiser",
                           "Reference Organiser " VERSION "\n"
                           "Technical Paper Organiser\n\n"
                           "Copyright 2023, Lyndon Hill\n"
                           "http://www.lyndonhill.com");
}

// Quit
void OrganiserMain::Quit()
{
  close();
}

void OrganiserMain::Settings()
{
  SettingsDialog *prefs = new SettingsDialog(this);
  prefs->SetPapersPaths(papersPaths);
  prefs->SetReadPapersPath(readPapersPath);
  prefs->SetMaxCitationAuthors(prefMaximumCiteAuthors);
  prefs->SetMaxCitationCharacters(prefMaximumCiteCharacters);

  prefs->SetDocViewerCLI(prefDOCViewer);
  prefs->SetODTViewerCLI(prefODTViewer);
  prefs->SetPDFViewerCLI(prefPDFViewer);
  prefs->SetPSViewerCLI(prefPSViewer);
  prefs->SetTextViewerCLI(prefTextViewer);
  prefs->SetBackupViewerCLI(prefBackupViewer);

  if(prefs->exec() == QDialog::Accepted)
  {
    papersPaths  = prefs->GetPapersPaths();
    if(prefs->GetPathsChanged()) ScanPaperPaths();

    readPapersPath   = prefs->GetReadPapersPath();
    prefMaximumCiteAuthors    = prefs->GetMaxCitationAuthors();
    prefMaximumCiteCharacters = prefs->GetMaxCitationCharacters();

    prefDOCViewer    = prefs->GetDocViewerCLI();
    prefODTViewer    = prefs->GetODTViewerCLI();
    prefPDFViewer    = prefs->GetPDFViewerCLI();
    prefPSViewer     = prefs->GetPSViewerCLI();
    prefTextViewer   = prefs->GetTextViewerCLI();
    prefBackupViewer = prefs->GetBackupViewerCLI();

    saveSettings();
  }
}

// Search dialog
void OrganiserMain::Search()
{
  SearchDialog *search = new SearchDialog(this);
  search->SetRecords(db.database);
  search->SetPapersRead(readPapersPath);
  int result = search->exec();

  // Convert results from search to records
  searchResults.clear();
  QStringList search_citations = search->GetResults();
  for(int c = 0; c < search_citations.size(); c++)
  {
    for(int r = 0; r < db.database.size(); r++)
    {
      if(db.database[r].citation == search_citations[c])
      {
        searchResults.push_back(db.database[r]);
      }
    }
  }

  if(!searchResults.empty() && (result == QDialog::Accepted))
  {
    if(ui->viewCombo->currentIndex() != 4)
      ui->viewCombo->setCurrentIndex(4); // set to results
    else
      UpdateView();
  }
}

// Name/rename database
void OrganiserMain::DatabaseName()
{
  // Open a databasenamedialog
  DatabaseNameDialog *name_dialog = new DatabaseNameDialog(this);
  name_dialog->SetName(db.databaseName);

  if(name_dialog->exec() == QDialog::Accepted)
  {
    db.databaseName = name_dialog->GetName();
    ui->refList->clearSelection();
    showDatabaseDetails();
  }
}

// Set records
void OrganiserMain::setRecords(const QVector<PaperMeta> &recs)
{
  records = recs;
  UpdateView();

  ui->busyWidget->stop();
}

// Set to current review
void OrganiserMain::showDetailsForReview(int index)
{
  /* Old version:
   *
  bool examine_search = false;  // looking at searchResults

  if(ui->viewCombo->currentIndex() == 4)
    examine_search = true;

  if(index < 0) return;
  if(examine_search && (index >= searchResults.size())) return;
  else if(index >= db.database.size()) return;

  clearDetails();

  // Get current review

  PaperMeta current_record;
  if(examine_search)
    current_record = searchResults[index];
  else
    current_record = db.database[index];

  currentReviewText = current_record.review;
  displayFormattedDetails(current_record);

  // Look to see if paper is available
  findPaper(index);
  */

  /* New version: */

  if(index < 0) return;
  if(index >= db.database.size()) return;  // impossible

  bool has_details = true;

  PaperMeta current_record;

  switch(ui->viewCombo->currentIndex())
  {
    case 0:
    // Papers
    current_record = db.database[index];
    break;

    case 1:
    // New papers
    if(index >= newPapers.size()) return;
    // No record exists, only the paper file
    // Create a pseudo record on the fly to allow user to create a new review:
    current_record.paperPath = newPapers[index]; // need to find path in correct new papers directory(?)
    current_record.pseudo = true;
    has_details = true;
    break;

    case 2:
    // Reviewed papers TODO
    break;

    case 3:
    // Complete reviews TODO
    break;

    case 4:
    // Search results
std::cerr << "There are " << searchResults.size() << " results\n";
    if(index >= searchResults.size()) return;
std::cerr << "Showing search result " << index << "\n";
    current_record = searchResults[index];
    break;

    default:
    // None of the above!
    return;
    break;
  }

  clearDetails();

  if(has_details)
  {
    currentReviewText = current_record.review;
    displayFormattedDetails(current_record);

    // Look to see if paper is available
    findPaper(index);
  }
}

// Citations in database that are repeated
void OrganiserMain::setDuplicates(const QStringList &duplicates)
{
  duplicateRefs = duplicates;
}

void OrganiserMain::showDatabaseDetails() const
{
  if(!db.databaseName.isEmpty())
  {
    ui->refList->clearSelection();
    QString database_details("<html><body><p><b>Database Name:</b> ");
    database_details.append(db.databaseName).append("<br>");
    database_details.append(QString("<b>Filename:</b> %1<br><br>").arg(lastDatabaseFilename));
    database_details.append(QString("%1 records.<br>").arg(db.database.size()));
    database_details.append(QString("%1 new papers.<br><br>").arg(newPapers.size()));

    database_details.append(QString("%1 tags").arg(tags.size()));
    if(tags.empty()) database_details.append(".");
    else             database_details.append(": ");
    for(int t = 0; t < tags.size(); t++) database_details.append(QString(" %1").arg(tags[t]));

    database_details.append("</body></html>");
    ui->detailsViewer->setText(database_details);
  }
}

// Select a review
void OrganiserMain::selectItem()
{
  RecordListItem *current = dynamic_cast<RecordListItem *>(ui->refList->currentItem());
  if(current)
  {
    showDetailsForReview(current->GetRecordIndex());
    ui->editButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
  }
  else
  {
    ui->detailsViewer->clear();
  }
}

// Select the given citation
void OrganiserMain::selectCitation(const QString &cite)
{
  for(int c = 0; c < ui->refList->count(); c++)
  {
    QListWidgetItem *item = ui->refList->item(c);
    if(item->text() == cite)
    {
      ui->refList->setCurrentItem(item);
      item->setSelected(true);
      break;
    }
  }
}

// User clicked on link in review
void OrganiserMain::gotoLinkedReview(const QUrl &link)
{
  // Simple search
  QListWidgetItem *review_item = nullptr;

  for(int c = 0; c < ui->refList->count(); c++)
  {
    if(ui->refList->item(c)->text() == link.fileName())
    {
      review_item = ui->refList->item(c);
      break;
    }
  }

  if(review_item)
  {
    ui->refList->setCurrentItem(review_item);  // will set current review
  }
  else
  {
    // TODO if it is a link don't clear the widget
    // TODO consider ui->detailsViewer->setOpenExternalLinks(true) instead of this
    if(link.toString().startsWith("http"))
      QProcess::startDetached("xdg-open", QStringList(link.toString()));
    else
    {
      std::cerr << "Linked review does not exist!\n";
      ui->detailsViewer->setHtml(tr("<html><body><p><b>Review for %1 does not exist!</b></p></body></html").arg(link.fileName()));
    }
  }
}

void OrganiserMain::loadSettings()
{
  QSettings settings("lyndonhill.com", "RefOrg");
  settings.beginGroup("main");

  int psize = settings.beginReadArray("paper_paths");
  for(int i = 0; i < psize; i++)
  {
    settings.setArrayIndex(i);
    papersPaths << settings.value("paper_dir").toString();
  }
  readPapersPath = settings.value("read_papers_dir", "").toString();
  settings.endArray();

  lastDatabaseFilename      = settings.value("last_database", "").toString();
  lastExportedHTML          = settings.value("exported_html", "").toString();
  lastEnteredReview         = QDate::fromString(settings.value("last_review_date", "Thu Jul 1 2021").toString());
  settings.endGroup();

  settings.beginGroup("citations");
  prefMaximumCiteAuthors    = settings.value("max_authors", 5).toInt();
  prefMaximumCiteCharacters = settings.value("max_characters", 32).toInt();
  settings.endGroup();

  settings.beginGroup("viewer");
#if __APPLE__
  prefDOCViewer    = settings.value("doc",    "/Applications/LibreOffice.app/Contents/MacOS/soffice --writer").toString();
  prefODTViewer    = settings.value("odt",    "/Applications/LibreOffice.app/Contents/MacOS/soffice --writer").toString();
  prefPDFViewer    = settings.value("pdf",    "open").toString();
  prefPSViewer     = settings.value("ps",     "open").toString();
  prefTextViewer   = settings.value("text",   "open").toString();
  prefBackupViewer = settings.value("backup", "open").toString();
#elif _WIN32
  prefDOCViewer    = settings.value("doc",    "explorer").toString();
  prefODTViewer    = settings.value("odt",    "soffice --writer").toString();
  prefPDFViewer    = settings.value("pdf",    "explorer").toString();
  prefPSViewer     = settings.value("ps",     "explorer").toString();
  prefTextViewer   = settings.value("text",   "explorer").toString();
  prefBackupViewer = settings.value("backup", "explorer").toString();
#else
  prefDOCViewer    = settings.value("doc",    "/usr/lib/libreoffice/program/soffice.bin --writer").toString();
  prefODTViewer    = settings.value("odt",    "/usr/lib/libreoffice/program/soffice.bin --writer").toString();
  prefPDFViewer    = settings.value("pdf",    "okular").toString();
  prefPSViewer     = settings.value("ps",     "okular").toString();
  prefTextViewer   = settings.value("text",   "kate").toString();
  prefBackupViewer = settings.value("backup", "xdg-open").toString();
#endif
  settings.endGroup();
}

void OrganiserMain::saveSettings()
{
  QSettings settings("lyndonhill.com", "RefOrg");
  settings.beginGroup("main");

  settings.beginWriteArray("paper_paths");
  for(int i = 0; i < papersPaths.count(); i++)
  {
    settings.setArrayIndex(i);
    settings.setValue("paper_dir", papersPaths[i]);
  }
  settings.setValue("read_papers_dir", readPapersPath);
  settings.endArray();

  settings.setValue("last_database", lastDatabaseFilename);
  settings.setValue("exported_html", lastExportedHTML);
  settings.setValue("last_review_date", lastEnteredReview.toString());
  settings.endGroup();

  settings.beginGroup("citations");
  settings.setValue("max_authors", prefMaximumCiteAuthors);
  settings.setValue("max_characters", prefMaximumCiteCharacters);
  settings.endGroup();

  settings.beginGroup("viewer");
  settings.setValue("doc",    prefDOCViewer);
  settings.setValue("odt",    prefODTViewer);
  settings.setValue("pdf",    prefPDFViewer);
  settings.setValue("ps",     prefPSViewer);
  settings.setValue("text",   prefTextViewer);
  settings.setValue("backup", prefBackupViewer);
  settings.endGroup();
}


// Parse database for all tags used
void OrganiserMain::buildTagList()
{
  tags.clear();

  for(int r = 0; r < db.database.size(); r++)
  {
    if(db.database[r].tags.isEmpty()) continue;

    QStringList record_tags = db.database[r].tags.split(",", Qt::SkipEmptyParts);
    for(int t = 0; t < record_tags.size(); t++)
    {
      bool present = false;
      for(int i = 0; i < tags.size(); i++)
      {
        if(tags[i] == record_tags[t])
        {
          present = true;
          break;
        }
      }

      if(!present)
        tags.push_back(record_tags[t]);
    }
  }

  tags.sort();
  ui->tagFilterCombo->clear();
  ui->tagFilterCombo->addItems(tags);
  ui->tagFilterCombo->setEnabled(!tags.empty());
}

// Scan the paths to where papers are stored and make a list of new papers
void OrganiserMain::ScanPaperPaths()
{
  newPapers.clear();

  QFlags<QDir::Filter>   myspec = QDir::AllEntries | QDir::NoDotAndDotDot;
  QFlags<QDir::SortFlag> mysort = QDir::IgnoreCase | QDir::Name;

  QVector<QFileInfoList> current_dir_listing;

  for(int p = 0; p < papersPaths.size(); p++)
  {
    std::cerr << "Search path " << papersPaths[p].toStdString() << "\n";
    QDir path(papersPaths[p]);

    QFileInfoList listing;
    listing = path.entryInfoList(myspec, mysort);
    current_dir_listing.push_back(listing);

    while(!current_dir_listing.isEmpty())
    {
      listing = current_dir_listing.back();
      current_dir_listing.pop_back();

      if(listing.size())
      {
        QList<QFileInfo>::Iterator fi;
        fi = listing.begin();

        std::cerr << "Search point " << fi->absoluteFilePath().toStdString() << "\n";

        while(fi != listing.end())
        {
          if(fi->isDir())
          {
            QString temp_path = fi->absoluteFilePath();
            std::cerr << "Adding path " << temp_path.toStdString() << " to search list\n";
            QDir temp_dir(temp_path);
            QFileInfoList temp_listing = temp_dir.entryInfoList(myspec, mysort);
            current_dir_listing.push_back(temp_listing);
          }
          else
            newPapers.push_back(fi->absoluteFilePath());  // TODO only add if recognised paper file type

          ++fi;
        }
      }
    }
  }
}

void OrganiserMain::displayFormattedDetails(const PaperMeta &meta_record)
{
  if(meta_record.pseudo)
  {
    QString formatted_text = QString("<html><body><p><b>%1</b></p><p>New paper: no details in database.</p></body></html>").arg(meta_record.paperPath.section('/', -1));
    ui->detailsViewer->setHtml(formatted_text);
    return;
  }

  QString formatted_text("<html><body><p><b>");
  formatted_text.append(meta_record.title).append("</b><br>");

  QStringList authors_list = ParseAuthors(meta_record.authors);
  for(int a = 0; a < authors_list.size(); a++)
  {
    formatted_text.append(authors_list[a]);
    if(a == authors_list.size()-2)
      formatted_text.append(" and ");
    else if((authors_list.size() > 1) && (a < authors_list.size()-1))
      formatted_text.append(", ");
  }
  formatted_text.append("</p>");
  formatted_text.append("<hr>");

  if(!meta_record.publication.isEmpty())
  {
    formatted_text.append(QString("Appeared in %1").arg(meta_record.publication));
    if(!meta_record.volume.isEmpty()) formatted_text.append(tr(", vol. %1").arg(meta_record.volume));
    if(!meta_record.issue.isEmpty()) formatted_text.append(tr(", no. %1").arg(meta_record.issue));

    if(!meta_record.pageStart.isEmpty() && !meta_record.pageEnd.isEmpty())
      formatted_text.append(QString(", pp%1-%2").arg(meta_record.pageStart, meta_record.pageEnd));
    else if(!meta_record.pageStart.isEmpty())
      formatted_text.append(QString(" at p%1").arg(meta_record.pageStart));

    formatted_text.append("<br>");
  }
  else if(meta_record.thesis != ThesisType::UnknownThesisType)
  {
    switch(meta_record.thesis)
    {
      case ThesisType::Doctorate:
      formatted_text.append(QString("Doctoral thesis."));
      break;

      case ThesisType::Masters:
      formatted_text.append(QString("Masters thesis."));
      break;

      case ThesisType::Bachelors:
      formatted_text.append(QString("Bachelors thesis."));
      break;

      case ThesisType::College:
      formatted_text.append(QString("College thesis."));
      break;

      case ThesisType::UnknownThesisType:
      // Nothing to do here
      break;
    }

    formatted_text.append("<br>");
  }
  else if(meta_record.venue == VenueType::Report)
  {
    formatted_text.append(QString("Report.<br>"));
  }

  formatted_text.append(meta_record.year);
  formatted_text.append("<br>");

  if(!meta_record.URL.isEmpty())
    formatted_text.append(QString("<a href=\"%1\">%1</a><br>").arg(meta_record.URL));

  if(!meta_record.doi.isEmpty())
    formatted_text.append(QString("<a href=\"https://doi.org/%1\">https://doi.org/%1</a><br>").arg(meta_record.doi));

  if(!meta_record.tags.isEmpty())
  {
    formatted_text.append("<br>");
    QString spaced_tags = meta_record.tags;
    spaced_tags.replace(",", ", ");
    formatted_text.append(QString("Tags: %1<br>").arg(spaced_tags));
  }

  if(meta_record.reviewDate.isValid())
    formatted_text.append(QString("<br><i>Review edited on %1.</i>").arg(meta_record.reviewDate.toString()));

  formatted_text.append("<hr>");
  formatted_text.append("<pre>");

  QString review_only(meta_record.review.toHtmlEscaped());
  if(!review_only.isEmpty())
  {
    review_only.replace(QRegularExpression("\\{([^}]*)\\}"), "[<a href=\"\\1\">\\1</a>]");
    formatted_text.append(review_only);
  }
  else
  {
    formatted_text.append(tr("Review not yet written"));
  }

  formatted_text.append("</pre>");

  // if(meta_record.reviewer.accept)  TODO detect review was set
  {
    formatted_text.append("<hr>");

    formatted_text.append(tr("<p>You rated this paper with the following scores:</p><br>"));
    formatted_text.append(tr("<table border=\"1\"><tr><th>Category</th><th>Rating</th></tr>"));
    formatted_text.append(tr("<tr><td>Suitability</td><td>%1</td></tr>").arg(meta_record.reviewer.suitability));
    formatted_text.append(tr("<tr><td>Technical Correctness</td><td>%1</td></tr>").arg(meta_record.reviewer.technicalCorrectness));
    formatted_text.append(tr("<tr><td>Novelty</td><td>%1</td></tr>").arg(meta_record.reviewer.novelty));
    formatted_text.append(tr("<tr><td>Clarity</td><td>%1</td></tr>").arg(meta_record.reviewer.clarity));
    formatted_text.append(tr("<tr><td>Relevance</td><td>%1</td></tr>").arg(meta_record.reviewer.relevance));
    formatted_text.append("</table><br>");

    if(!meta_record.reviewer.commentsToAuthors.isEmpty())
      formatted_text.append(tr("<p>Your comments to the authors:</p><p><i>%1</i></p>").arg(meta_record.reviewer.commentsToAuthors));

    if(!meta_record.reviewer.commentsToChairEditor.isEmpty())
      formatted_text.append(tr("<p>Your comments to the area chair:</p><p><i>%1</i></p>").arg(meta_record.reviewer.commentsToChairEditor));

    QString accept_as_string;
    switch(meta_record.reviewer.accept)
    {
      case Accept_Strong:
      accept_as_string = tr("Strong Accept");
      break;

      case Accept_Weak:
      accept_as_string = tr("Weak Accept");
      break;

      default:
      case Accept_Neutral:
      accept_as_string = tr("Neutral");
      break;

      case Reject_Weak:
      accept_as_string = tr("Weak Reject");
      break;

      case Reject_Strong:
      accept_as_string = tr("Strong Reject");
      break;
    }

    QString corrections;
    if(meta_record.reviewer.correctionsRequired) corrections = tr(" with corrections required");

    formatted_text.append(QString("<p>Your verdict was <b>%1</b>%2.</p>").arg(accept_as_string, corrections));
  }

  formatted_text.append("</body></html>");

  ui->detailsViewer->setHtml(formatted_text);
}

// Find paper for given index
void OrganiserMain::findPaper(int index)
{
  // FIXME support newPapers mode

  bool examine_search = false;  // looking at searchResults

  if(ui->viewCombo->currentIndex() == 4)
    examine_search = true;

  if(index < 0) return;
  if((examine_search) && (index >= searchResults.size())) return;
  else if(index >= db.database.size()) return;

  currentPaperPath.clear();
  ui->openPaperButton->setEnabled(false);

  if(examine_search)
  {
    // Search result
    if((!searchResults[index].paperPath.isEmpty()) && (QFile::exists(searchResults[index].paperPath)))
      currentPaperPath = searchResults[index].paperPath;
  }
  else if(ui->viewCombo->currentIndex() == 1)
  {
    // New papers
std::cerr << "Looking for file of new paper at " << newPapers[index].toStdString() << "\n";
    if((!newPapers[index].isEmpty()) && (QFile::exists(newPapers[index])))
      currentPaperPath = newPapers[index];
  }
  else
  {
    if((!db.database[index].paperPath.isEmpty()) && (QFile::exists(db.database[index].paperPath)))
      currentPaperPath = db.database[index].paperPath;
  }

  if(!currentPaperPath.isEmpty())
  {
    ui->openPaperButton->setEnabled(true);
    ui->paperPathLabel->setText(QDir::toNativeSeparators(currentPaperPath));
  }
  else
  {
    ui->openPaperButton->setToolTip("");
    ui->paperPathLabel->setText(tr("No paper found"));
  }
}

// Open the current paper
void OrganiserMain::openCurrentPaper()
{
  if(!currentPaperPath.isEmpty())
  {
    QString extension = currentPaperPath.section('.', -1).toLower();
    QString viewer;
    QStringList arguments;

    QString viewer_string = prefBackupViewer;

    if((extension == "doc") || (extension == "docx"))
    {
      viewer_string = prefDOCViewer;
    }
    else if(extension == "odt")
    {
      viewer_string = prefODTViewer;
    }
    else if(extension == "pdf")
    {
      viewer_string = prefPDFViewer;
    }
    else if((extension == "ps") || (currentPaperPath.section('.', -2, -1).toLower() == "ps.gz"))
    {
      viewer_string = prefPSViewer;
    }
    else if(extension == "txt")
    {
      viewer_string = prefTextViewer;
    }
    else
      viewer_string = prefBackupViewer; // FIXME dvi opened via backup viewer

    QStringList cli_components = viewer_string.split(" ");
    if(!cli_components.empty())
      viewer = cli_components[0];
    for(int c = 1; c < cli_components.size(); c++)
      arguments << cli_components[c];

    QString native_path = QDir::toNativeSeparators(currentPaperPath);
    arguments << native_path;

    bool result = QProcess::startDetached(viewer, arguments);
    if(!result)
    {
      QMessageBox::critical(this, tr("Open Paper"), tr("Failed to open viewer"));
    }
  }
}

// Last accessed paper path
void OrganiserMain::setLastPaperLocation(const QString &path)
{
  lastPaperPath = path.section("/", 0, -2);
}

// Show the review database status
void OrganiserMain::showStatus()
{
  // Clear all
  ui->openPaperButton->setEnabled(false);
  ui->detailsViewer->clear();
  ui->paperPathLabel->clear();

  // Get statistics

  // TODO: new papers; total reviews; completed reviews

  int total_reviews          = db.database.size();
  int completed_reviews      = 0;
  int papers_to_read         = newPapers.size();
  int papers_with_reviews    = 0;
  int papers_without_reviews = 0;


  for(int r = 0; r < db.database.size(); r++)
  {
    if(db.database[r].reader.finished) completed_reviews++;

    if(db.database[r].review.isEmpty())
      papers_without_reviews++;
    else
      papers_with_reviews++;
  }

  // Format status

  QString formatted_text("<html><body><p><h1>Reference Database Status</h1></p><br>");

  formatted_text.append("<table cellspacing=\"20\">");
  formatted_text.append(tr("<tr><th>Total records</th><td>%1</td></tr>").arg(total_reviews));
  formatted_text.append(tr("<tr><th>Papers with reviews</th><td>%1</td></tr>").arg(papers_with_reviews));
  formatted_text.append(tr("<tr><th>Reviews marked complete</th><td>%1</td></tr>").arg(completed_reviews));
  formatted_text.append(tr("<tr><th>Records missing review</th><td>%1</td></tr>").arg(papers_without_reviews));
  formatted_text.append(tr("<tr><th>New papers to be read</th><td>%1</td></tr>").arg(papers_to_read));

  formatted_text.append("</table>");

  QString rating;

  if(papers_with_reviews > 4000)
    rating = tr("Elite");
  else if(papers_with_reviews > 2000)
    rating = tr("Guru");
  else if(papers_with_reviews > 1500)
    rating = tr("Superstar");
  else if(papers_with_reviews > 1000)
    rating = tr("Master");
  else if(papers_with_reviews > 800)
    rating = tr("Expert");
  else if(papers_with_reviews > 500)
    rating = tr("Consultant");
  else if(papers_with_reviews > 200)
    rating = tr("Teacher");
  else if(papers_with_reviews > 100)
    rating = tr("Advisor");
  else if(papers_with_reviews > 50)
    rating = tr("Student");
  else if(papers_with_reviews > 20)
    rating = tr("Mostly Novice");
  else
    rating = tr("Novice");

  formatted_text.append("<hr>");
  formatted_text.append(tr("<p>Your database ranking is <i>%1</i>.</p>").arg(rating));

  if(!duplicateRefs.empty())
  {
    formatted_text.append("<hr><b>Warning! Duplicate References Detected</b><br>");

    for(int r = 1; r < db.database.size(); r++) // TODO should this be r+=2 ?
    {
      if(db.database[r].citation == db.database[r-1].citation)
      {
        formatted_text.append(QString("<p>Reference %1<br>").arg(db.database[r].citation));

        if(db.database[r-1].paperPath != db.database[r].paperPath)
        {
          formatted_text.append(QString("has papers at<br>%1 <b>and</b> %2<br>").arg(db.database[r-1].paperPath, db.database[r].paperPath));
        }

        formatted_text.append("</p>");

/*
        std::cerr << "Scanner found duplicate: " << new_records[r].citation.toStdString() << "\n"
                  << " A: " << new_records[r-1].reviewPath.toStdString() << " : " << new_records[r-1].paperPath.toStdString() << "\n"
                  << " B: " << new_records[r].reviewPath.toStdString() << " : " << new_records[r].paperPath.toStdString() << "\n";
*/
      }
    }
  }

  formatted_text.append("</body></html>");

  ui->detailsViewer->setHtml(formatted_text);
}

// Rescan and update view
void OrganiserMain::refresh()
{
  ScanPaperPaths();
  UpdateView();
}

// Rescan/refresh
void OrganiserMain::rescanPaths()
{
  // clear all
  ui->openPaperButton->setEnabled(false);
  ui->refList->clear();

  ScanPaperPaths();
}

// Change to a different view mode
void OrganiserMain::changeViewMode(int)
{
  UpdateView();
  clearDetails();
}

// Clear all tags in the filter
void OrganiserMain::clearTagFilters()
{
  ui->tagFilterEdit->clear();
}

// Change tag filter logic mode
void OrganiserMain::toggleTagFilterLogic()
{
  tagFilterAnd = !tagFilterAnd;

  if(tagFilterAnd)
    ui->tagFilterLogicButton->setText(tr("AND"));
  else
    ui->tagFilterLogicButton->setText(tr("OR"));

  UpdateView();
}

// Add the current tag selected in the combo to the filter list
void OrganiserMain::addCurrentTagFilter()
{
  QString current_tag = ui->tagFilterCombo->currentText();
  if(ui->tagFilterEdit->text().isEmpty())
    ui->tagFilterEdit->setText(current_tag);
  else
  {
    // Check chosen tag is not present already
    QStringList tag_filter = ui->tagFilterEdit->text().split(",", Qt::SkipEmptyParts);
    bool present = false;
    for(int t = 0; t < tag_filter.size(); t++)
    {
      if(tag_filter[t] == current_tag)
      {
        present = true;
        break;
      }
    }

    if(!present)
      ui->tagFilterEdit->setText(ui->tagFilterEdit->text() + "," + current_tag);
  }
}

// Edit an existing review
void OrganiserMain::editReview()
{
  if(!ui->refList->currentItem()) return;

  QString cite = ui->refList->currentItem()->text();
  PaperMeta meta;

  bool found = false;

  if(ui->viewCombo->currentIndex() == 1)
  {
    for(int r = 0; r < newPapers.size(); r++)
    {
      if(newPapers[r].section('/', -1) == cite)
      {
        meta.citation = cite;
        meta.year = "";
        meta.paperPath = newPapers[r];
        // Question: do we need to set pseudo to true?
        // TODO guess authors details and year from filename?
        found = true;
        break;
      }
    }
  }
  else
  {
    // Search database for citation
    for(int r = 0; r < db.database.size(); r++)
    {
      if(db.database[r].citation == cite)
      {
        meta = db.database[r];
        found = true;
        break;
      }
    }
  }

  if(!found)
  {
    std::cerr << "Could not find citation in database\n";
    return;
  }

  MetaDialog *edit_dialog = new MetaDialog(this);
  edit_dialog->setWindowTitle(tr("Edit Review"));
  edit_dialog->SetMeta(meta);
  edit_dialog->SetTags(tags);

  if(!lastPaperPath.isEmpty()) edit_dialog->SetPaperHuntDir(lastPaperPath);

  if(meta.paperPath.indexOf(readPapersPath) == 0)
    edit_dialog->SetPaperIngested();

  connect(edit_dialog, &MetaDialog::updatedDetails,     this,        &OrganiserMain::modifyOrNew);
  connect(edit_dialog, &MetaDialog::requestCitation,    this,        &OrganiserMain::generateKey);
  connect(edit_dialog, &MetaDialog::paperLocated,       this,        &OrganiserMain::setLastPaperLocation);
  connect(this,        &OrganiserMain::haveNewCitation, edit_dialog, &MetaDialog::SetCitation);

  edit_dialog->show();
}

// Create a new review
void OrganiserMain::newReview()
{
  MetaDialog *edit_dialog = new MetaDialog(this);
  edit_dialog->setWindowTitle(tr("New Review"));

  if(!lastPaperPath.isEmpty()) edit_dialog->SetPaperHuntDir(lastPaperPath);

  connect(edit_dialog, &MetaDialog::updatedDetails,     this,        &OrganiserMain::modifyOrNew);
  connect(edit_dialog, &MetaDialog::requestCitation,    this,        &OrganiserMain::generateKey);
  connect(edit_dialog, &MetaDialog::paperLocated,       this,        &OrganiserMain::setLastPaperLocation);
  connect(this,        &OrganiserMain::haveNewCitation, edit_dialog, &MetaDialog::SetCitation);

  edit_dialog->show();
}

// Create or modify a review
void OrganiserMain::modifyOrNew(PaperMeta &meta)
{
  // Set date
  meta.reviewDate = QDate::currentDate();

  // Add any new tags

  int tags_added = 0;
  QStringList meta_tags = meta.tags.split(",", Qt::SkipEmptyParts);
  for(int t = 0; t < meta_tags.size(); t++)
  {
    QString ttag = meta_tags[t].trimmed();

    if(tags.contains(ttag))
      continue;
    else
    {
      std::cerr << "Found a new tag, " << ttag.toStdString() << "\n";
      tags.push_back(ttag);
      tags_added++;
    }
  }

  // Always store tags as sorted
  meta_tags.sort(Qt::CaseInsensitive);
  meta.tags = meta_tags.join(",");

  // Move paper to read papers directory, if requested

  if(meta.ingest)
  {
    // Check path
    if(meta.paperPath.indexOf(readPapersPath) == 0)
      std::cerr << "Paper has already been ingested\n";
    else
    {
      // Check if paper with proposed name already exists at readPapersPath
      QString ext = meta.paperPath.section(".", -1);
      QString potential_file_name = readPapersPath + "/" + meta.citation + "." + ext;
      QFile potential_file(potential_file_name);
      if(potential_file.exists())
      {
        QMessageBox::warning(this, tr("Citation already exists?"),
                                   tr("Could not move paper to read papers directory"));
      }
      else
      {
        QFile current_paper(meta.paperPath);
        bool ingest_result = current_paper.rename(potential_file_name);
        if(!ingest_result)
        {
          QMessageBox::warning(this, tr("Failed to ingest paper"),
                                     tr("Could not move paper to read papers directory"));
        }
        else
          meta.paperPath = potential_file_name;
      }
    }
  }

  // Check if meta already in the database

  bool in_database = false;
  QString cite_searchterm;
  if(!meta.originalCitation.isEmpty() && (meta.originalCitation != meta.citation))
    cite_searchterm = meta.originalCitation;
  else
    cite_searchterm = meta.citation;

  for(int r = 0; r < db.database.size(); r++)
  {
    if((db.database[r].citation == cite_searchterm) && (!meta.citation.isEmpty()))
    {
      // In database -> update the record
      in_database = true;
      db.database[r]  = meta;
      break;
    }
  }

  if(!in_database)
  {
    // Not in database -> add to database
    db.database.push_back(meta);

    QDate current_date = QDate::currentDate();
    if(current_date > lastEnteredReview)
        lastEnteredReview = current_date;
  }

  db.Sort();
  if(tags_added) buildTagList();

  // Update search results if record was searched
  if(!searchResults.empty() && ui->viewCombo->currentIndex() == 4)
  {
    for(int r = 0; r < searchResults.size(); r++)
    {
      if((searchResults[r].citation == cite_searchterm) && (!meta.citation.isEmpty()))
      {
        // In results -> update the record
        searchResults[r]  = meta;
        break;
      }
    }
  }

  UpdateView();

  for(int r = 0; r < ui->refList->count(); r++)
  {
    QListWidgetItem *row_r = ui->refList->item(r);
    if(row_r->text() == meta.citation)
    {
      ui->refList->setCurrentItem(row_r);
      break;
    }
  }
}

// Delete a review
void OrganiserMain::deleteReview()
{   
  if(!ui->refList->currentItem()) return;

  QString selected_citation = ui->refList->currentItem()->text();

  // Check with user
  int ret = QMessageBox::critical(this, tr("Delete Paper"),
                                        tr("Are you sure you want to delete the paper %1").arg(selected_citation),
                                        QMessageBox::Ok | QMessageBox::Cancel);

  if(ret == QMessageBox::Ok)
  {
    int item_to_remove = ui->refList->currentRow();

    if(ui->viewCombo->currentIndex() != 0)
    {
      // Current view is not unfiltered database
      for(int r = 0; r < db.database.size(); r++)
      {
        if(db.database[r].citation == selected_citation)
        {
          item_to_remove = r;
          break;
        }
      }
    }

    db.database.remove(item_to_remove);

    clearDetails();

    // remove from list of reviews
    QListWidgetItem *item = ui->refList->takeItem(ui->refList->currentRow());
    if(item) delete item;

    buildTagList();
    UpdateView();
  }
}

// Clear details from main window widgets
void OrganiserMain::clearDetails()
{
  currentReviewText.clear();
  currentPaperPath.clear();

  ui->detailsViewer->clear();
  ui->paperPathLabel->clear();
}

// Import review files
void OrganiserMain::ImportReviews()
{
  QStringList source_files = QFileDialog::getOpenFileNames(this, tr("Import Records"), QDir::homePath());
  if(source_files.empty()) return;

  // Attempt to import each file
  for(int f = 0; f < source_files.size(); f++)
  {
    QString review;
    if(LoadReview(source_files[f], review))
    {
      QString authors, title, year;
      ParseReview(source_files[f], review, authors, title, year);
      TrimReviewHeader(review);
      PaperMeta meta;
      QString citation = source_files[f].section('/', -1); // trim path
      if(checkCitationExists(citation))
      {
        qDebug() << "Citation " << citation.toUtf8().constData() << " already exists in the database\n";
      }

      meta.citation = citation;
      meta.authors  = authors;
      meta.title    = title;
      meta.year     = year;
      meta.review   = review.trimmed();

      // Find corresponding paper - TODO improve this block
      QString paper_candidate = readPapersPath + "/" + citation;
      if(QFile::exists(paper_candidate + ".doc"))
        meta.paperPath = paper_candidate + ".doc";
      else if(QFile::exists(paper_candidate + ".docx"))
        meta.paperPath = paper_candidate + ".docx";
      else if(QFile::exists(paper_candidate + ".dvi"))
        meta.paperPath = paper_candidate + ".dvi";
      else if(QFile::exists(paper_candidate + ".odt"))
        meta.paperPath = paper_candidate + ".odt";
      else if(QFile::exists(paper_candidate + ".pdf"))
        meta.paperPath = paper_candidate + ".pdf";
      else if(QFile::exists(paper_candidate + ".ps"))
        meta.paperPath = paper_candidate + ".ps";
      else if(QFile::exists(paper_candidate + ".ps.gz"))
        meta.paperPath = paper_candidate + ".ps.gz";
      else if(QFile::exists(paper_candidate + ".txt"))
        meta.paperPath = paper_candidate + ".txt";

      QFileInfo finfo(source_files[f]);
      meta.reviewDate = finfo.lastModified().date();
      db.database.push_back(meta);
    }
  }

  db.Sort();
  UpdateView();
}


// Export the reviews as a single page HTML file
void OrganiserMain::ExportHTML()
{
  QString outfile = QFileDialog::getSaveFileName(this, tr("Export to..."), QDir::homePath(), tr("Hypertext (*.html)"));
  if(outfile.isEmpty()) return;

  // Open file
  std::ofstream op(outfile.toUtf8().constData());
  if(op.bad())
  {
    QMessageBox::warning(this, tr("Reference Organiser"),
                         tr("Could not open file for export.\n"),
                         QMessageBox::Cancel);
    return;
  }

  ui->busyWidget->start();

  // Output header
  op << "<html>\n <head>\n";
  op << " <style>\
          .reference { padding:5px; margin-bottom:20px } \
          .title { font-size:18px; } \
          .authors { font-size:14px; } \
          .year { font-size:10px; } \
          .review { border-left:8px solid #dadada; margin:10px; padding:8px; background-color:#fafafa } \
          .citation { color:#646464; } \
          pre { white-space: pre-wrap; } \
          </style>\n";
  op << " </head>\n <body>\n";

  // Iterate through each reference with a review
  for(int r = 0; r < db.database.size(); r++)
  {
    QString review;
    QString authors, title, year;

    review  = db.database[r].review;
    authors = db.database[r].authors;
    title   = db.database[r].title;
    year    = db.database[r].year;

    review = review.trimmed();  // remove extra whitespace at start and end of string

    QStringList authors_list = ParseAuthors(authors);
    QString authors_formatted;
    for(int a = 0; a < authors_list.size(); a++)
    {
      authors_formatted.append(authors_list[a]);
      if(a == authors_list.size()-2)
        authors_formatted.append(" and ");
      else if((authors_list.size() > 1) && (a < authors_list.size()-1))
        authors_formatted.append(", ");
    }

    // Output the formatted review
    op << " <div class=\"reference\">\n";
    op << "  <div class=\"title\">"    << title.toUtf8().constData() << "</div>\n";
    op << "  <div class=\"authors\">"  << authors_formatted.toUtf8().constData() << "</div>\n";
    op << "  <div class=\"year\">"     << year.toUtf8().constData() << "</div>\n";

    if(!review.isEmpty())
      op << "  <div class=\"review\">\n  <pre>\n" << review.toUtf8().constData() << "</pre>\n  </div>\n";

    op << "  <div class=\"citation\">" << db.database[r].citation.toUtf8().constData() << "</div>\n";
    if(!db.database[r].paperPath.isEmpty())
      op << "  <a href=\"file://" << db.database[r].paperPath.toUtf8().constData() << "\">paper</a>";
    op << " </div>\n";
  }

  // Output footer
  op << " <hr><small>Exported from Reference Organiser</small>";
  op << " </body>\n</html>\n";

  // Close file
  op.close();

  lastExportedHTML = outfile;
  ui->busyWidget->stop();
}

// Save the paper database
bool OrganiserMain::SaveDatabaseAs()
{
  QString documents_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  if(documents_path.isEmpty()) documents_path = QDir::homePath();

  // Get filename to save to
  QString filename = QFileDialog::getSaveFileName(this, tr("Database filename"),
                                                        QString("%1/papers.rodb").arg(documents_path),
                                                        tr("Database files (*.rodb *.xml)"));
  if(filename.isEmpty())
    return(false);

  return(db.Save(filename.toUtf8().constData()));
}

// New database
bool OrganiserMain::NewDatabase()
{
  CreateDatabaseDialog *dialog = new CreateDatabaseDialog(this);
  if(dialog->exec() == QDialog::Accepted)
  {
    if(!db.database.empty())
      saveDatabase();

    QString new_name = dialog->GetDatabaseName();
    QString new_file = dialog->GetDatabaseFilename();
    lastDatabaseFilename = new_file;
    db.New(new_name);
    tags.clear();
    searchResults.clear();

    UpdateView();
    return(true);
  }

  return(false);
}

// Load previous database
void OrganiserMain::LoadPreviousDatabase()
{
  // Save current database first, if necessary
  if(!db.database.empty())
  {
    if(!saveDatabase())
    {
      QMessageBox::critical(this, tr("Could not save database"), tr("Current database could not be saved."));
      return;
    }
  }

  // Clear current database

  ui->editButton->setEnabled(false);
  ui->deleteButton->setEnabled(false);
  ui->openPaperButton->setEnabled(false);
  ui->refList->clear();
  ui->detailsViewer->clear();
  ui->paperPathLabel->clear();
  db.database.clear();
  db.databaseName.clear();
  tags.clear();

  QString documents_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  if(documents_path.isEmpty()) documents_path = QDir::homePath();

  // Get filename to load
  QString filename = QFileDialog::getOpenFileName(this, tr("Load Database"),
                                                          documents_path,
                                                          tr("Database files (*.rodb *.xml)"));
  if(filename.isEmpty())
    return;

  // FIXME  save current database and clear!

  if(!db.Load(filename.toUtf8().constData()))
  {
    qDebug() << "Failed to open database " << filename << "\n";
    lastDatabaseFilename.clear();
    db.database.clear();
    db.databaseName.clear();
    // FIXME any work added now probably won't get saved
  }
  else
  {
    qDebug() << "Loaded database name " << db.databaseName << "\n";
    lastDatabaseFilename = filename;
    buildTagList();
    UpdateView();
  }
}

// Close
void OrganiserMain::closeEvent(QCloseEvent *)
{
  saveDatabase();
  saveSettings();
}

void OrganiserMain::startup()
{
  bool success = false;

  // Load last used database or start a new one if none available

  if(!lastDatabaseFilename.isEmpty())
  {
    ui->busyWidget->start();
    success = db.Load(lastDatabaseFilename.toUtf8().constData());
    ui->busyWidget->stop();

    if(!success)
      qDebug() << "Could not load database <" << lastDatabaseFilename.toUtf8().constData() << ">\n";
  }

  if(!success)
  {
    // Clear any failed database load
    lastDatabaseFilename.clear();
    db.database.clear();
    db.databaseName.clear();

    if(!NewDatabase())
    {
      /* No database set so use default filename

      QString documents_path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
      if(documents_path.isEmpty()) documents_path = QDir::homePath();
      QString fallback_filename = QString("%1/%2").arg(documents_path).arg("papers.rodb");
       */
      QMessageBox::critical(this, tr("No database"), tr("You <b>must</b> create a new database otherwise all work will be lost."));
    }
  }

  buildTagList();
  ScanPaperPaths();
  UpdateView();
}

// Save current database
bool OrganiserMain::saveDatabase()
{
  return(db.Save(lastDatabaseFilename.toUtf8().constData()));
}

// Generate a citation key for the given authors and year
void OrganiserMain::generateKey(const QString &authors, const QString &year)
{
  QString key;
  QString cat_authors;

  // Extract surnames and concatenate
  // Note: this won't do a good job for people with multiple surnames unless they are hyphenated

  // Replace "and" with comma; need space before and after in case name contains "and"
  QString author_names = authors;
  author_names.replace(tr(" and "), ", ", Qt::CaseInsensitive);
  author_names.replace("-", ""); // remove hyphens, usually in double barrelled names

  QStringList names = author_names.split(',', Qt::SkipEmptyParts);
  if(names.empty()) return;

  if(names.size() <= prefMaximumCiteAuthors)
  {
    for(int n = 0; n < names.size(); n++)
    {
      QStringList name_elements = names[n].split(' ', Qt::SkipEmptyParts); // assume family name last
      cat_authors.append(name_elements.last());
    }
    key = cat_authors + year;
  }

  if((names.size() > prefMaximumCiteAuthors) || (key.size() > prefMaximumCiteCharacters))
  {
    // Single author name in case of too many authors or too long string
    QStringList name_elements = names[0].split(' ', Qt::SkipEmptyParts); // assume family name last

    key = name_elements.last() + tr("EtAl") + year;

    if(!checkCitationExists(key))
    {
      // -> Try AlphaBravoCharlie2000 = ABC2000
      key.clear();
      for(int a = 0; a < names.size(); a++)
      {
        QStringList name_elements = names[a].split(' ', Qt::SkipEmptyParts); // assume family name last
        key.append(name_elements.last()[0]);
      }
      key = key + year;
    }
  }

  // Check if key already exists in records

  bool key_in_use = checkCitationExists(key);

  // Add a, b, c etc postfix to key if it already exists
  // TODO: incorporate title/venue in key if suitable postfix not possible

  char key_ext = 'a';

  while(key_in_use)
  {
    if(!checkCitationExists(key + key_ext))  // TODO must test this
    {
      key += key_ext;
      break;
    }

    key_ext++;

    if(key_ext > 'z')  // This is really unlikely - more than 26 similar citations!
    {
      key += '_';
      key_ext = 'a';
    }
  }

  emit haveNewCitation(key);
}

// Checks if citation is in use
bool OrganiserMain::checkCitationExists(const QString &text)
{
  for(int i = 0; i < db.database.size(); i++)
  {
    if(db.database[i].citation == text)
    {
      return(true);
    }
  }

  return(false);
}
