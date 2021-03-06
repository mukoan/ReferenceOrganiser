/**
 * @file   organisermain.cpp
 * @brief  Main window
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#include "organisermain.h"
#include "ui_organisermain.h"
#include "settingsdialog.h"
#include "reviewedit.h"
#include "searchdialog.h"
#include "reviewparser.h"
#include "recordlistitem.h"

#include <iostream>
#include <algorithm>
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
#include <QRegExp>
#include <QStandardPaths>

OrganiserMain::OrganiserMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::OrganiserMain)
{
  ui->setupUi(this);

  qRegisterMetaType<QVector<PaperRecord>>("QVector<PaperRecord>");

  QIcon myicon;
  QPixmap pix1x(":/Assets/reforg.png");
  QPixmap pix2x(":/Assets/reforg@2x.png");
  pix2x.setDevicePixelRatio(2);

  myicon.addPixmap(pix1x);
  myicon.addPixmap(pix2x);
  setWindowIcon(myicon);

  scanThread = nullptr;

  loadSettings();

  connect(ui->actionRefreshView,  &QAction::triggered,              this, &OrganiserMain::refreshView);
  connect(ui->actionAbout,        &QAction::triggered,              this, &OrganiserMain::About);
  connect(ui->actionQuit,         &QAction::triggered,              this, &OrganiserMain::Quit);

  connect(ui->actionExport_HTML,  &QAction::triggered,              this, &OrganiserMain::ExportHTML);
  connect(ui->actionPreferences,  &QAction::triggered,              this, &OrganiserMain::Settings);
  connect(ui->actionStatus,       &QAction::triggered,              this, &OrganiserMain::showStatus);

  ui->actionAbout->setMenuRole(QAction::AboutRole);
  ui->actionPreferences->setMenuRole(QAction::PreferencesRole);
  ui->actionQuit->setMenuRole(QAction::QuitRole);

  connect(ui->openPaperButton,    &QPushButton::released,           this, &OrganiserMain::openCurrentPaper);
  connect(ui->newReviewButton,    &QPushButton::released,           this, &OrganiserMain::newReview);
  connect(ui->editButton,         &QPushButton::released,           this, &OrganiserMain::editReview);
  connect(ui->deleteButton,       &QPushButton::released,           this, &OrganiserMain::deleteReview);
  connect(ui->searchButton,       &QPushButton::released,           this, &OrganiserMain::Search);

  connect(ui->refList,            &QListWidget::itemSelectionChanged, this, &OrganiserMain::selectItem);

  connect(ui->viewCombo,          static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                                                                    this, &OrganiserMain::changeViewMode);
  connect(ui->detailsViewer,      &QTextBrowser::anchorClicked,     this, &OrganiserMain::gotoLinkedReview);

  refreshView();
}

OrganiserMain::~OrganiserMain()
{
  delete ui;
}

// Build simple list of papers with reviews
void OrganiserMain::ScanRecords()
{
  ui->busyWidget->start();
  records.clear();

  scanThread = new QThread;
  scanThread->setObjectName("RefOrg-Scan");

  scanner = new ReviewScanner;
  scanner->SetPaths(reviewsPaths, papersPaths);

  scanner->moveToThread(scanThread);
  connect(scanThread, SIGNAL(started()), scanner, SLOT(process()));
  connect(scanner, SIGNAL(results(const QVector<PaperRecord> &)), this, SLOT(setRecords(const QVector<PaperRecord> &)));
  connect(scanner, SIGNAL(foundDuplicates(const QStringList &)), this, SLOT(setDuplicates(const QStringList &)));
  // connect(ui->scanButton, SIGNAL(clicked()), scanner, SLOT(halt()));
  connect(scanner, SIGNAL(finished()), scanner, SLOT(deleteLater()));
  connect(scanThread, SIGNAL(finished()), scanThread, SLOT(deleteLater()));
  scanThread->start();
}

// Update view - use viewCombo and records to update view shown
void OrganiserMain::UpdateView()
{
  // Clear all

  ui->openPaperButton->setEnabled(false);
  ui->refList->clear();
  ui->detailsViewer->clear();
  ui->reviewPathLabel->clear();

  int selected_records = 0;

  if(ui->viewCombo->currentIndex() == 5)
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
    for(int r = 0; r < records.size(); r++)
    {
      switch(ui->viewCombo->currentIndex())
      {
        case 0: // reviews
        if(!records[r].reviewPath.isEmpty())
        {
          RecordListItem *element = new RecordListItem(ui->refList, r);
          element->setText(records[r].citation);
          selected_records++;
        }
        break;

        case 1: // papers
        if(!records[r].paperPath.isEmpty())
        {
          RecordListItem *element = new RecordListItem(ui->refList, r);
          element->setText(records[r].citation);
          selected_records++;
        }
        break;

        case 2: // reviewed papers
        if((!records[r].reviewPath.isEmpty()) && (!records[r].paperPath.isEmpty()))
        {
          RecordListItem *element = new RecordListItem(ui->refList, r);
          element->setText(records[r].citation);
          selected_records++;
        }
        break;

        case 3: // unreviewed papers
        if((!records[r].paperPath.isEmpty()) && (records[r].reviewPath.isEmpty()))
        {
          RecordListItem *element = new RecordListItem(ui->refList, r);
          element->setText(records[r].citation);
          selected_records++;
        }
        break;

        case 4: // all
        RecordListItem *element = new RecordListItem(ui->refList, r);
        element->setText(records[r].citation);
        selected_records++;
        break;
      }
    }
  }

  ui->numberPapersLabel->setText(QString("%1").arg(selected_records));
}

// About dialog
void OrganiserMain::About()
{
  QMessageBox::about(this, "About Reference Organiser",
                           "Reference Organiser " VERSION "\n"
                           "Technical Paper Organiser\n\n"
                           "Copyright 2018, Lyndon Hill\n"
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
  prefs->SetReviewsPaths(reviewsPaths);
  prefs->SetMaxCitationAuthors(prefMaximumCiteAuthors);
  prefs->SetMaxCitationCharacters(prefMaximumCiteCharacters);

  prefs->SetPDFViewerCLI(prefPDFViewer);
  prefs->SetPSViewerCLI(prefPSViewer);
  prefs->SetDocViewerCLI(prefDOCViewer);
  prefs->SetTextViewerCLI(prefTextViewer);
  prefs->SetBackupViewerCLI(prefBackupViewer);

  if(prefs->exec() == QDialog::Accepted)
  {
    papersPaths  = prefs->GetPapersPaths();
    reviewsPaths = prefs->GetReviewsPaths();

    prefReviewOutputPath = prefs->GetChosenReviewPathIndex();

    if(reviewsPaths.empty())
    {
      reviewsPaths << QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
      prefReviewOutputPath = 0;
    }

    if(prefs->GetPathsChanged()) refreshView();  // rescan

    prefMaximumCiteAuthors    = prefs->GetMaxCitationAuthors();
    prefMaximumCiteCharacters = prefs->GetMaxCitationCharacters();

    prefPDFViewer    = prefs->GetPDFViewerCLI();
    prefPSViewer     = prefs->GetPSViewerCLI();
    prefDOCViewer    = prefs->GetDocViewerCLI();
    prefTextViewer   = prefs->GetTextViewerCLI();
    prefBackupViewer = prefs->GetBackupViewerCLI();

    saveSettings();
  }
}

// Search dialog
void OrganiserMain::Search()
{
  SearchDialog *search = new SearchDialog(this);
  search->SetRecords(records);
  int result = search->exec();

  // Convert results from search to records
  searchResults.clear();
  QStringList search_citations = search->GetResults();
  for(int c = 0; c < search_citations.size(); c++)
  {
    for(int r = 0; r < records.size(); r++)
    {
      if(records[r].citation == search_citations[c])
      {
        searchResults.push_back(records[r]);
      }
    }
  }

  if(!searchResults.empty() && (result == QDialog::Accepted))
  {
    if(ui->viewCombo->currentIndex() != 5)
      ui->viewCombo->setCurrentIndex(5); // set to results
    else
      refreshView();
  }
}

// Set records
void OrganiserMain::setRecords(const QVector<PaperRecord> &recs)
{
  records = recs;
  UpdateView();

  // QCoreApplication::processEvents();
  ui->busyWidget->stop();
}

// Set to current review
void OrganiserMain::showDetailsForReview(const QString fname)
{
  if(fname.isEmpty()) return;

  clearDetails();

  // Get current review

  QString path_to_review;
  bool have_review = false;

  int r;
  for(r = 0; r < records.size(); r++)
  {
    if(records[r].citation == fname)
    {
      path_to_review = records[r].reviewPath;
      break;
    }
  }

  if(!path_to_review.isEmpty())
  {
    FILE *fp;
    if((fp = fopen(path_to_review.toUtf8().constData(), "rb")))
    {
      fseek(fp, 0, SEEK_END);
      size_t data_size = ftell(fp);
      rewind(fp);

      char *data = (char *)malloc(data_size+1);
      if(!data)
      {
        fclose(fp);
        return;
      }

      fread(data, data_size, 1, fp);
      fclose(fp);

      data[data_size] = 0;

      currentReviewText = QString(data);

      // Parse review (no database)
      displayFormattedDetails(fname, currentReviewText);
      have_review = true;
      free(data);
    }
  }

  // Look to see if paper is available
  findPaper(fname);

  if(have_review)
  {
    path_to_review = QDir::toNativeSeparators(path_to_review);
    ui->reviewPathLabel->setText(path_to_review);
  }
  else
    ui->reviewPathLabel->setText(tr("No review available"));
}

// Set to current review
void OrganiserMain::showDetailsForReview(int index)
{
  bool examine_search = false;  // looking at searchResults

  if(ui->viewCombo->currentIndex() == 5)
    examine_search = true;

  if(index < 0) return;
  if(examine_search && (index >= searchResults.size())) return;
  else if((index < 0) || (index >= records.size())) return;

  clearDetails();

  // Get current review

  QString path_to_review;
  if(examine_search)
    path_to_review = searchResults[index].reviewPath;
  else
    path_to_review = records[index].reviewPath;

  bool have_review = false;

  if(!path_to_review.isEmpty())
  {
    FILE *fp;
    if((fp = fopen(path_to_review.toUtf8().constData(), "rb")))
    {
      fseek(fp, 0, SEEK_END);
      size_t data_size = ftell(fp);
      rewind(fp);

      char *data = (char *)malloc(data_size+1);
      if(!data)
      {
        fclose(fp);
        return;
      }

      fread(data, data_size, 1, fp);
      fclose(fp);

      data[data_size] = 0;

      currentReviewText = QString(data);

      // Parse review (no database)
      displayFormattedDetails(path_to_review, currentReviewText);
      have_review = true;
      free(data);
    }
  }

  // Look to see if paper is available
  findPaper(index);

  if(have_review)
  {
    path_to_review = QDir::toNativeSeparators(path_to_review);
    ui->reviewPathLabel->setText(path_to_review);
  }
  else
    ui->reviewPathLabel->setText(tr("No review available"));
}

// Citations in database that are repeated
void OrganiserMain::setDuplicates(const QStringList &duplicates)
{
  duplicateRefs = duplicates;
}

// Select a review
void OrganiserMain::selectItem()
{
  RecordListItem *current = dynamic_cast<RecordListItem *>(ui->refList->currentItem());
  if(current)
  {
    showDetailsForReview(current->GetRecordIndex());
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
    ui->refList->setCurrentItem(review_item); // will set current review
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
  settings.endArray();
  int rsize = settings.beginReadArray("review_paths");
  for(int i = 0; i < rsize; i++)
  {
    settings.setArrayIndex(i);
    reviewsPaths << settings.value("review_dir").toString();
  }
  settings.endArray();

  prefReviewOutputPath = settings.value("output_path_idx", 0).toInt();

  if(reviewsPaths.empty())
  {
    reviewsPaths << QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
    prefReviewOutputPath = 0;
  }

  lastExportedHTML = settings.value("exported_html", "").toString();
  settings.endGroup();

  settings.beginGroup("citations");
  prefMaximumCiteAuthors = settings.value("max_authors", 5).toInt();
  prefMaximumCiteCharacters = settings.value("max_characters", 32).toInt();
  settings.endGroup();

  settings.beginGroup("viewer");
#if __APPLE__
  prefPDFViewer    = settings.value("pdf",    "open").toString();
  prefPSViewer     = settings.value("ps",     "open").toString();
  prefDOCViewer    = settings.value("doc",    "/Applications/LibreOffice.app/Contents/MacOS/soffice --writer").toString();
  prefTextViewer   = settings.value("text",   "open").toString();
  prefBackupViewer = settings.value("backup", "open").toString();
#elif _WIN32
  prefPDFViewer    = settings.value("pdf",    "explorer").toString();
  prefPSViewer     = settings.value("ps",     "explorer").toString();
  prefDOCViewer    = settings.value("doc",    "explorer").toString();
  prefTextViewer   = settings.value("text",   "explorer").toString();
  prefBackupViewer = settings.value("backup", "explorer").toString();
#else
  prefPDFViewer    = settings.value("pdf",    "okular").toString();
  prefPSViewer     = settings.value("ps",     "okular").toString();
  prefDOCViewer    = settings.value("doc",    "/usr/lib/libreoffice/program/soffice.bin --writer").toString();
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
  settings.endArray();
  settings.beginWriteArray("review_paths");
  for(int i = 0; i < reviewsPaths.count(); i++)
  {
    settings.setArrayIndex(i);
    settings.setValue("review_dir", reviewsPaths[i]);
  }
  settings.endArray();

  settings.setValue("output_path_idx", prefReviewOutputPath);

  settings.setValue("exported_html", lastExportedHTML);
  settings.endGroup();

  settings.beginGroup("citations");
  settings.setValue("max_authors", prefMaximumCiteAuthors);
  settings.setValue("max_characters", prefMaximumCiteCharacters);
  settings.endGroup();

  settings.beginGroup("viewer");
  settings.setValue("pdf", prefPDFViewer);
  settings.setValue("ps", prefPSViewer);
  settings.setValue("doc", prefDOCViewer);
  settings.setValue("text", prefTextViewer);
  settings.setValue("backup", prefBackupViewer);
  settings.endGroup();
}

void OrganiserMain::ScanPaths()
{
  for(int p = 0; p < reviewsPaths.size(); p++)
  {
    QDir path(reviewsPaths[p]);

    QFlags<QDir::Filter> myspec = QDir::AllEntries | QDir::NoDotAndDotDot;
    QFlags<QDir::SortFlag> mysort = QDir::IgnoreCase | QDir::Name;

    QFileInfoList listing;
    listing = path.entryInfoList(myspec, mysort);

    if(listing.size())
    {
      ui->numberPapersLabel->setText(QString::number(listing.size()));

      QList<QFileInfo>::Iterator fi;
      fi = listing.begin();

      while(fi != listing.end())
      {
        QString fname = fi->fileName();
        QListWidgetItem *element = new QListWidgetItem(ui->refList);
        element->setText(fname);

        fi++;
      }
    }
  }
}

// Display formatted text for review
void OrganiserMain::displayFormattedDetails(const QString &filename, const QString &review_text)
{
  QString authors, title, year;
  ParseReview(filename, review_text, authors, title, year);

  QString formatted_text("<html><body><p><b>");
  formatted_text.append(title).append("</b><br>");

  QStringList authors_list = ParseAuthors(authors);
  for(int a = 0; a < authors_list.size(); a++)
  {
    formatted_text.append(authors_list[a]);
    if(a == authors_list.size()-2)
      formatted_text.append(" and ");
    else if((authors_list.size() > 1) && (a < authors_list.size()-1))
      formatted_text.append(", ");
  }
  formatted_text.append("</p>");
  formatted_text.append("<small>");
  formatted_text.append(year);
  formatted_text.append("</small>");

  formatted_text.append("<hr>");
  formatted_text.append("<pre>");

  QString review_only(review_text);
  TrimReviewHeader(review_only);
  review_only.replace(QRegExp("\\{([^}]*)\\}"), "[<a href=\"\\1\">\\1</a>]");
  formatted_text.append(review_only);
  formatted_text.append("</pre>");

  formatted_text.append("</body></html>");

  ui->detailsViewer->setHtml(formatted_text);
}

// Find paper for given file reference
void OrganiserMain::findPaper(const QString &filename)
{
  currentPaperPath.clear();
  ui->openPaperButton->setEnabled(false);

  for(int r = 0; r < records.size(); r++)
  {
    if(records[r].citation == filename)
    {
      if(QFile::exists(records[r].paperPath))
      {
        currentPaperPath = records[r].paperPath;
        break;
      }
    }
  }

  if(!currentPaperPath.isEmpty())
  {
    ui->openPaperButton->setEnabled(true);
    ui->openPaperButton->setToolTip(QDir::toNativeSeparators(currentPaperPath));
  }
  else
  {
    ui->openPaperButton->setToolTip("");
    // std::cerr << "No paper found for " << filename.toStdString() << "\n";
  }
}

// Find paper for given index
void OrganiserMain::findPaper(int index)
{
  bool examine_search = false;  // looking at searchResults

  if(ui->viewCombo->currentIndex() == 5)
    examine_search = true;

  if(index < 0) return;
  if((examine_search) && (index >= searchResults.size())) return;
  else if(index >= records.size()) return;

  currentPaperPath.clear();
  ui->openPaperButton->setEnabled(false);

  if(examine_search)
  {
    if(QFile::exists(searchResults[index].paperPath))
      currentPaperPath = searchResults[index].paperPath;
  }
  else
  {
    if(QFile::exists(records[index].paperPath))
      currentPaperPath = records[index].paperPath;
  }

  if(!currentPaperPath.isEmpty())
  {
    ui->openPaperButton->setEnabled(true);
    ui->openPaperButton->setToolTip(QDir::toNativeSeparators(currentPaperPath));
  }
  else
  {
    ui->openPaperButton->setToolTip("");
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

    if(extension == "doc")
    {
      viewer_string = prefDOCViewer;
    }
    else if(extension == "txt")
    {
      viewer_string = prefTextViewer;
    }
    else if(extension == "ps")
    {
      viewer_string = prefPSViewer;
    }
    else if(extension == "pdf")
    {
      viewer_string = prefPDFViewer;
    }
    else if(currentPaperPath.section('.', -2, -1).toLower() == "ps.gz")
    {
      viewer_string = prefPSViewer;
    }

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

// Show the review database status
void OrganiserMain::showStatus()
{
  // Clear all
  ui->openPaperButton->setEnabled(false);
  ui->detailsViewer->clear();
  ui->reviewPathLabel->clear();

  // Get statistics

  int papers_on_disk = 0;
  int total_reviews  = 0;
  int reviews_without_papers = 0;
  int papers_with_reviews = 0;
  int papers_without_reviews = 0;

  for(int r = 0; r < records.size(); r++)
  {
    if(!records[r].reviewPath.isEmpty())
    {
      total_reviews++;
      if(records[r].paperPath.isEmpty())
        reviews_without_papers++;
      else
      {
        papers_with_reviews++;
        papers_on_disk++;
      }
    }
    else
    {
      if(!records[r].paperPath.isEmpty())
      {
        papers_without_reviews++;
        papers_on_disk++;
      }
    }
  }

  // Format status

  QString formatted_text("<html><body><p><h1>Reference Database Status</h1></p><br>");

  formatted_text.append("<table cellspacing=\"20\">");
  formatted_text.append(tr("<tr><th>Papers on disk</th><td>%1</td></tr>").arg(papers_on_disk));
  formatted_text.append(tr("<tr><th> ... of which reviewed</th><td>%1</td></tr>").arg(papers_with_reviews));
  formatted_text.append(tr("<tr><th>Papers to be reviewed</th><td>%1</td></tr>").arg(papers_without_reviews));
  formatted_text.append(tr("<tr><th>Total reviews</th><td>%1</td></tr>").arg(total_reviews));
  formatted_text.append(tr("<tr><th>Reviews without papers</th><td>%1</td></tr>").arg(reviews_without_papers));

  formatted_text.append("</table>");


  if((papers_on_disk > 40) && (total_reviews > 20))
  {
    QString rating = tr("(unrated)");
    float frating = (float)(papers_with_reviews*100)/(papers_on_disk);

    int irating = (int)(frating);
    irating += (reviews_without_papers*40)/(total_reviews);  // reviews should have some merit even when missing paper
    irating += (papers_without_reviews*5)/(papers_on_disk);  // having a paper but not reading it has small merit

    if(irating >= 90)
      rating = tr("Elite");
    else if(irating > 75)
      rating = tr("Guru");
    else if(irating > 60)
      rating = tr("Expert");
    else if(irating > 40)
      rating = tr("Consultant");
    else if(irating > 20)
      rating = tr("Advisor");
    else
      rating = tr("Novice");

    formatted_text.append("<hr>");
    formatted_text.append(tr("<p>Your database is rated <i>%1</i> quality.</p>").arg(rating));
  }


  if(!duplicateRefs.empty())
  {
    formatted_text.append("<hr><b>Warning! Duplicate References Detected</b><br>");

    for(int r = 1; r < records.size(); r++)
    {
      if(records[r].citation == records[r-1].citation)
      {
        formatted_text.append(QString("<p>Reference %1<br>").arg(records[r].citation));

        if(records[r-1].reviewPath != records[r].reviewPath)
        {
          formatted_text.append(QString(" has reviews at<br>%1 <b>and</b> %2<br>").arg(records[r-1].reviewPath).arg(records[r].reviewPath));
        }

        if(records[r-1].paperPath != records[r].paperPath)
        {
          formatted_text.append(QString("has papers at<br>%1 <b>and</b> %2<br>").arg(records[r-1].paperPath).arg(records[r].paperPath));
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

// Rescan/refresh
void OrganiserMain::rescanPaths()
{
  // clear all
  ui->openPaperButton->setEnabled(false);
  ui->refList->clear();

  ScanPaths();
}

// (Re)Scan records and update view
void OrganiserMain::refreshView()
{
  ScanRecords();
  UpdateView();
}

// Change to a different view mode
void OrganiserMain::changeViewMode(int)
{
  UpdateView();
  clearDetails();
}

// Edit an existing review
void OrganiserMain::editReview()
{
  if(!ui->refList->currentItem()) return;

  if((prefReviewOutputPath < 0) || (reviewsPaths.count() <= 0))
  {
    QMessageBox::warning(this, tr("Edit Review"), tr("A directory for storing reviews must be configured in Preferences"));
    return;
  }

  QString cite, review, authors, title, year;
  cite   = ui->refList->currentItem()->text();
  review = currentReviewText;
  ParseReview(cite, currentReviewText, authors, title, year);

  ReviewEdit *review_diag = new ReviewEdit(false, this);

  review_diag->SetAuthors(authors);
  review_diag->SetCitationKey(cite);

  QStringList lines = review.split('\n');
  QStringList::iterator it = lines.begin();

  if(!title.isEmpty())
  {
    it = lines.erase(it);                        // remove first line
    if(!authors.isEmpty()) it = lines.erase(it); // remove second line
    if((*it == "\n") || (it->isEmpty())) it = lines.erase(it);        // remove blank line

    review.clear();
    while(it != lines.end())
    {
      review = review + *it + "\n";
      ++it;
    }
  }

  // Set full path to citation being edited
  for(int r = 0; r < records.size(); r++)
  {
    if(records[r].citation == cite)
    {
      review_diag->SetReviewPath(records[r].reviewPath);
      break;
    }
  }

  review_diag->SetReviewText(review);
  review_diag->SetTitle(title);
  review_diag->SetYear(year);
  review_diag->SetMaximumAuthors(prefMaximumCiteAuthors);
  review_diag->SetMaximumCharacters(prefMaximumCiteCharacters);
  review_diag->SetRecords(records);

  review_diag->SetTargetDir(reviewsPaths[prefReviewOutputPath]); // In case citation changed

  review_diag->setWindowTitle(tr("Edit %1").arg(cite));

  connect(review_diag, &ReviewEdit::reviewSaved, this, &OrganiserMain::saveDone);

  review_diag->show();
}

// Create a new review
void OrganiserMain::newReview()
{  
  if((prefReviewOutputPath < 0) || (reviewsPaths.count() <= 0))
  {
    QMessageBox::warning(this, tr("New Review"), tr("A directory for storing reviews must be configured in Preferences"));
    return;
  }

  ReviewEdit *review_diag = new ReviewEdit(true, this);
  review_diag->SetMaximumAuthors(prefMaximumCiteAuthors);
  review_diag->SetMaximumCharacters(prefMaximumCiteCharacters);
  review_diag->SetRecords(records);


  QString citation = ui->refList->currentItem()->text();
  if(!citation.isEmpty())
  {
    // Check if current item has review

    bool has_review = false;
    for(int r = 0; r < records.size(); r++)
    {
      if(records[r].citation == citation)
      {
        has_review = !records[r].reviewPath.isEmpty();
        break;
      }
    }

    // Unreviewed papers or all papers but this paper has no review
    if((ui->viewCombo->currentIndex() == 3) ||
       ((ui->viewCombo->currentIndex() == 1) && (!has_review)))
    {
      // If review doesn't exist yet then use paper name as citation
      review_diag->SetCitationKey(citation);
    }
  }

  review_diag->SetTargetDir(reviewsPaths[prefReviewOutputPath]);
  review_diag->setWindowTitle(tr("New Review"));

  connect(review_diag, &ReviewEdit::reviewSaved, this, &OrganiserMain::saveDone);

  review_diag->show();
}

// Delete a review
void OrganiserMain::deleteReview()
{
  if(!ui->refList->currentItem()) return;

  QString selected_citation = ui->refList->currentItem()->text();

  // Find path to review
  QString full_path;

  for(int r = 0; r < reviewsPaths.size(); r++)
  {
    if(QFile::exists(reviewsPaths[r] + QDir::separator() + selected_citation))
    {
      full_path = reviewsPaths[r] + QDir::separator() + selected_citation;
      break;
    }
  }

  // Check with user
  int ret = QMessageBox::critical(this, tr("Delete Review"),
                                        tr("Are you sure you want to delete the review %1").arg(selected_citation),
                                        QMessageBox::Ok | QMessageBox::Cancel);

  if(ret == QMessageBox::Ok)
  {
    // Delete file
    bool removed = QFile::remove(full_path);
    if(removed)
    {
      clearDetails();

      // remove from list of reviews
      int r = ui->refList->currentRow();
      QListWidgetItem *item = ui->refList->takeItem(r);
      if(item) delete item;
    }
  }
}

// Edit/New review dialog closed after saving
void OrganiserMain::saveDone(bool refresh)
{
  QListWidgetItem *current = ui->refList->currentItem();
  QString current_citation;
  if(current)
    current_citation = current->text();

  clearDetails();

  if(refresh)
  {
    // Rescan reviews list and select citation

    ui->refList->clearSelection();
    refreshView();    // update reviews list

    if(!current_citation.isEmpty())
      selectCitation(current_citation);
  }
  else
  {
    // Show details after edit
    if(!current_citation.isEmpty())
      showDetailsForReview(current_citation);
  }
}

// Clear details from main window widgets
void OrganiserMain::clearDetails()
{
  currentReviewText.clear();
  currentPaperPath.clear();

  ui->detailsViewer->clear();
  ui->reviewPathLabel->clear();
}

/* Import files; e.g. review files, BibTex, etc
void OrganiserMain::Import()
{
  // TODO not implemented
}
*/

// Export the reviews as a single page HTML file
void OrganiserMain::ExportHTML()
{
  QString outfile = QFileDialog::getSaveFileName(this, tr("Export to..."), QDir::homePath(), tr("Hypertext (*.html)"));
  if(outfile.isEmpty()) return;

  // Open file
  std::ofstream op(outfile.toUtf8().constData());
  if(op.bad())
  {
    std::cerr << "Error: Could not open file for export\n";
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
  for(int r = 0; r < records.size(); r++)
  {
    if(records[r].reviewPath.isEmpty()) continue; // skip references without review

    // Load the review
    QString review;

    FILE *fp;
    if((fp = fopen(records[r].reviewPath.toUtf8().constData(), "rb")))
    {
      fseek(fp, 0, SEEK_END);
      size_t data_size = ftell(fp);
      rewind(fp);

      char *data = (char *)malloc(data_size+1);
      if(!data)
      {
        fclose(fp);
        return;
      }

      fread(data, data_size, 1, fp);
      fclose(fp);

      data[data_size] = 0;

      review = data;

      free(data);
    }

    QString authors, title, year;
    ParseReview(records[r].reviewPath, review, authors, title, year);

    if(review.isEmpty())
    {
      std::cerr << "review at " << records[r].reviewPath.toStdString() << " is empty\n";
      break;
    }

    TrimReviewHeader(review);   // remove title and authors
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
    op << "  <div class=\"review\">\n  <pre>\n" << review.toUtf8().constData() << "</pre>\n  </div>\n";
    op << "  <div class=\"citation\">" << records[r].citation.toUtf8().constData() << "</div>\n";
    if(!records[r].paperPath.isEmpty())
      op << "  <a href=\"file://" << records[r].paperPath.toUtf8().constData() << "\">paper</a>";
    op << " </div>\n";

    // Output to file

  }

  // Output footer
  op << " <hr><small>Exported from Reference Organiser</small>";
  op << " </body>\n</html>\n";

  // Close file
  op.close();

  lastExportedHTML = outfile;
  ui->busyWidget->stop();
}
