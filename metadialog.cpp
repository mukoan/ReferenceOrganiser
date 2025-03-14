/**
 * @file   metadialog.cpp
 * @brief  Edit paper metadata
 * @author Lyndon Hill
 * @date   2020.10.11
 */

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>

#include "duplicatesviewer.h"
#include "metadialog.h"
#include "ui_metadialog.h"

#include <iostream>

MetaDialog::MetaDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::MetaDialog), isEdit(false)
{
  ui->setupUi(this);

  paperHuntDir = QDir::homePath();

  saveContinueButton = ui->buttonBox->addButton(tr("Save and Continue"), QDialogButtonBox::ApplyRole);
  saveContinueButton->setToolTip(tr("Save review but do not close"));
  saveContinueButton->setEnabled(false);

  QPushButton *save_button = ui->buttonBox->button(QDialogButtonBox::Save);
  if(save_button) save_button->setEnabled(false);

  ui->reviewDate->setText(QDate::currentDate().toString(QLocale::system().dateFormat(QLocale::ShortFormat)));

  connect(ui->venueCombo,        QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MetaDialog::setVenueType);
  connect(ui->generateButton,    &QPushButton::released,          this, &MetaDialog::generateCite);
  connect(ui->citationEdit,      &QLineEdit::textChanged,         this, &MetaDialog::citationUpdated);
  connect(ui->paperSelectButton, &QPushButton::released,          this, &MetaDialog::locatePaper);
  connect(ui->paperPathEdit,     &QLineEdit::textEdited,          this, &MetaDialog::checkDenyMove);
  connect(ui->tagCombo,          &QComboBox::currentIndexChanged, this, &MetaDialog::tagSelected);
  connect(ui->tagAddButton,      &QToolButton::released,          this, &MetaDialog::addTag);
  connect(ui->tagClearButton,    &QToolButton::released,          this, &MetaDialog::clearTags);
  connect(ui->buttonBox,         &QDialogButtonBox::rejected,     this, &MetaDialog::requestToCancel);
  connect(ui->buttonBox,         &QDialogButtonBox::accepted,     this, &MetaDialog::saveAndClose);
  connect(saveContinueButton,    &QPushButton::released,          this, &MetaDialog::saveOnly);
}

MetaDialog::~MetaDialog()
{
  delete ui;
}

// Set the meta data
void MetaDialog::SetMeta(const PaperMeta &paper_details)
{
  ui->citationEdit->setText(paper_details.citation);
  ui->authorsEdit->setText(paper_details.authors);
  ui->titleEdit->setText(paper_details.title);
  ui->reviewEdit->setPlainText(paper_details.review);
  ui->paperPathEdit->setText(paper_details.paperPath);
  ui->publicationEdit->setText(paper_details.publication);
  ui->volumeEdit->setText(paper_details.volume);
  ui->issueEdit->setText(paper_details.issue);
  ui->monthEdit->setText(paper_details.month);
  ui->yearEdit->setText(paper_details.year);
  ui->datesEdit->setText(paper_details.dates);
  ui->pageStartEdit->setText(paper_details.pageStart);
  ui->pageEndEdit->setText(paper_details.pageEnd);
  ui->urlEdit->setText(paper_details.URL);
  ui->institutionEdit->setText(paper_details.institution);
  ui->locationEdit->setText(paper_details.location);
  ui->publisherEdit->setText(paper_details.publisher);
  ui->isbnEdit->setText(paper_details.ISBN);
  ui->doiEdit->setText(paper_details.doi);
  ui->reviewDate->setText(paper_details.reviewDate.toString(QLocale::system().dateFormat(QLocale::ShortFormat)));
  ui->tagsEdit->setText(paper_details.tags);                                 // comma separated tags

  checkDenyMove(paper_details.paperPath);  // Should UI offer to move paper to read papers directory

  switch(paper_details.venue)
  {
    case VenueType::Journal:            ui->venueCombo->setCurrentIndex(0);  break;
    case VenueType::Conference:         ui->venueCombo->setCurrentIndex(1);  break;
    case VenueType::Symposium:          ui->venueCombo->setCurrentIndex(2);  break;
    case VenueType::Book:               ui->venueCombo->setCurrentIndex(3);  break;
    case VenueType::Preprint:           ui->venueCombo->setCurrentIndex(4);  break;
    case VenueType::Thesis:             ui->venueCombo->setCurrentIndex(5);  break;
    case VenueType::Report:             ui->venueCombo->setCurrentIndex(6);  break;
    case VenueType::SelfPublished:      ui->venueCombo->setCurrentIndex(7);  break;
    case VenueType::NotPublished:       ui->venueCombo->setCurrentIndex(8);  break;
    case VenueType::NoVenue:            ui->venueCombo->setCurrentIndex(9);  break;
    case VenueType::UnknownVenue:       ui->venueCombo->setCurrentIndex(10); break;
  }

  switch(paper_details.thesis)
  {
    case ThesisType::Doctorate:         ui->thesisCombo->setCurrentIndex(0); break;
    case ThesisType::Masters:           ui->thesisCombo->setCurrentIndex(1); break;
    case ThesisType::Bachelors:         ui->thesisCombo->setCurrentIndex(2); break;
    case ThesisType::College:           ui->thesisCombo->setCurrentIndex(3); break;
    case ThesisType::UnknownThesisType: ui->thesisCombo->setCurrentIndex(4); break;
  }

  ui->completeCheck->setChecked(paper_details.reader.finished);
  ui->understandingSpin->setValue(paper_details.reader.understanding);
  ui->ratingSpin->setValue(paper_details.reader.rating);

  ui->suitabilitySpin->setValue(paper_details.reviewer.suitability);
  ui->correctnessSpin->setValue(paper_details.reviewer.technicalCorrectness);
  ui->noveltySpin->setValue(paper_details.reviewer.novelty);
  ui->claritySpin->setValue(paper_details.reviewer.clarity);
  ui->relevanceSpin->setValue(paper_details.reviewer.relevance);

  ui->commentsToAuthorsEdit->setPlainText(paper_details.reviewer.commentsToAuthors);
  ui->commentsToChairEdit->setPlainText(paper_details.reviewer.commentsToChairEditor);
  ui->correctionsRequiredCheck->setChecked(paper_details.reviewer.correctionsRequired);
  ui->ratedCheck->setChecked(paper_details.reviewed);

  ui->suitabilitySpin->setEnabled(paper_details.reviewed);
  ui->correctnessSpin->setEnabled(paper_details.reviewed);
  ui->noveltySpin->setEnabled(paper_details.reviewed);
  ui->claritySpin->setEnabled(paper_details.reviewed);
  ui->relevanceSpin->setEnabled(paper_details.reviewed);
  ui->commentsToAuthorsEdit->setEnabled(paper_details.reviewed);
  ui->commentsToChairEdit->setEnabled(paper_details.reviewed);
  ui->correctionsRequiredCheck->setEnabled(paper_details.reviewed);
  ui->acceptCombo->setEnabled(paper_details.reviewed);

  switch(paper_details.reviewer.accept)
  {
    case Accept_Strong:
    ui->acceptCombo->setCurrentIndex(0);
    break;

    case Accept_Weak:
    ui->acceptCombo->setCurrentIndex(1);
    break;

    default:
    case Accept_Neutral:
    ui->acceptCombo->setCurrentIndex(2);
    break;

    case Reject_Weak:
    ui->acceptCombo->setCurrentIndex(3);
    break;

    case Reject_Strong:
    ui->acceptCombo->setCurrentIndex(4);
    break;
  }

  originalCitation = paper_details.citation;
  originalCopy = paper_details;
  isEdit = true;
}

// Get the meta data
PaperMeta MetaDialog::GetMeta()
{
  PaperMeta my_paper;

  my_paper.citation    = ui->citationEdit->text();
  my_paper.review      = ui->reviewEdit->toPlainText();
  my_paper.paperPath   = ui->paperPathEdit->text();

  switch(ui->venueCombo->currentIndex())
  {
    case  0: my_paper.venue = VenueType::Journal;       break;
    case  1: my_paper.venue = VenueType::Conference;    break;
    case  2: my_paper.venue = VenueType::Symposium;     break;
    case  3: my_paper.venue = VenueType::Book;          break;
    case  4: my_paper.venue = VenueType::Preprint;      break;
    case  5: my_paper.venue = VenueType::Thesis;        break;
    case  6: my_paper.venue = VenueType::Report;        break;
    case  7: my_paper.venue = VenueType::SelfPublished; break;
    case  8: my_paper.venue = VenueType::NotPublished;  break;
    case  9: my_paper.venue = VenueType::NoVenue;       break;
    case 10: my_paper.venue = VenueType::UnknownVenue;  break;
  }

  my_paper.authors     = ui->authorsEdit->text().simplified();
  my_paper.title       = ui->titleEdit->text().simplified();
  my_paper.publication = ui->publicationEdit->text().simplified();
  my_paper.volume      = ui->volumeEdit->text();
  my_paper.issue       = ui->issueEdit->text();
  my_paper.month       = ui->monthEdit->text();
  my_paper.year        = ui->yearEdit->text();
  my_paper.dates       = ui->datesEdit->text();
  my_paper.pageStart   = ui->pageStartEdit->text();
  my_paper.pageEnd     = ui->pageEndEdit->text();

  // Remove any additional whitespace; but there may be multiple URLs separated
  // by commas
  QString url          = ui->urlEdit->text();
  if(url.contains(',')) {
    // Trim individual URLs
    QStringList separate_urls = url.split(',');
    for(auto &u : separate_urls)
      u = u.trimmed();
    url = separate_urls.join(',');
    my_paper.URL = url;
  } else {
    my_paper.URL = url.trimmed();
  }

  switch(ui->thesisCombo->currentIndex())
  {
    case 0: my_paper.thesis = ThesisType::Doctorate;         break;
    case 1: my_paper.thesis = ThesisType::Masters;           break;
    case 2: my_paper.thesis = ThesisType::Bachelors;         break;
    case 3: my_paper.thesis = ThesisType::College;           break;
    case 4: my_paper.thesis = ThesisType::UnknownThesisType; break;
  }

  my_paper.institution      = ui->institutionEdit->text().simplified();
  my_paper.location         = ui->locationEdit->text().simplified();
  my_paper.publisher        = ui->publisherEdit->text().simplified();
  my_paper.ISBN             = ui->isbnEdit->text();
  my_paper.doi              = ui->doiEdit->text().trimmed();

  my_paper.reviewDate       = QDate::currentDate();
  my_paper.tags             = ui->tagsEdit->text().simplified();

  my_paper.originalCitation = originalCitation;
  originalCitation.clear();

  my_paper.reader.finished      = ui->completeCheck->isChecked();
  my_paper.reader.understanding = ui->understandingSpin->value();
  my_paper.reader.rating        = ui->ratingSpin->value();

  my_paper.reviewed             = ui->ratedCheck->isChecked();

  if(ui->acceptCombo->currentIndex() != 2 || ui->correctionsRequiredCheck->isChecked() ||
     (ui->suitabilitySpin->value() > 1) || (ui->correctnessSpin->value() > 1) || (ui->noveltySpin->value() != 1) ||
     (ui->claritySpin->value() != 1) || (ui->relevanceSpin->value() != 1) ||
     (!ui->commentsToAuthorsEdit->document()->isEmpty()) || (!ui->commentsToChairEdit->document()->isEmpty()))
  {
      switch(ui->acceptCombo->currentIndex())
      {
        case 0:
        my_paper.reviewer.accept = Accept_Strong;
        break;

        case 1:
        my_paper.reviewer.accept = Accept_Weak;
        break;

        default:
        case 2:
        my_paper.reviewer.accept = Accept_Neutral;
        break;

        case 3:
        my_paper.reviewer.accept = Reject_Weak;
        break;

        case 4:
        my_paper.reviewer.accept = Reject_Strong;
        break;
      }

    my_paper.reviewer.suitability           = ui->suitabilitySpin->value();
    my_paper.reviewer.technicalCorrectness  = ui->correctnessSpin->value();
    my_paper.reviewer.novelty               = ui->noveltySpin->value();
    my_paper.reviewer.clarity               = ui->claritySpin->value();
    my_paper.reviewer.relevance             = ui->relevanceSpin->value();
    my_paper.reviewer.correctionsRequired   = ui->correctionsRequiredCheck->isChecked();
    my_paper.reviewer.commentsToAuthors     = ui->commentsToAuthorsEdit->toPlainText();
    my_paper.reviewer.commentsToChairEditor = ui->commentsToChairEdit->toPlainText();
  }

  my_paper.ingest           = ui->movePapersCheck->isChecked();

  return(my_paper);
}

// Set the pre existing tags
void MetaDialog::SetTags(const QStringList &tags)
{
  ui->tagCombo->addItems(tags);
  clearTags();
}

// Disable move paper option
void MetaDialog::SetPaperIngested()
{
  ui->movePapersCheck->setEnabled(false);
}

// Check if paper should be moved
bool MetaDialog::GetIngestPaper()
{
  return(ui->movePapersCheck->isChecked());
}

// Set font to use in editor
void MetaDialog::SetEditFont(const QString &font_name, int font_size)
{
  if(font_size > 0)
  {
    ui->reviewEdit->setFont(QFont(font_name, font_size));
  }
}

// Set the citation: use if not using SetMeta() or used when generating citation
void MetaDialog::SetCitation(const QString &cite)
{
  ui->citationEdit->setText(cite);
}

// Let the dialog know aboui possible duplicates
void MetaDialog::SuggestDuplicates(const QVector<PaperMeta> &duplicates)
{
  // Display in potential duplicates in dialog

  DuplicatesViewer *viewer = new DuplicatesViewer(this);
  viewer->SetDuplicates(duplicates);
  viewer->show();
}

// Venue type was changed
void MetaDialog::setVenueType(int new_venue)
{
  ui->datesEdit->setEnabled((new_venue == 1) || (new_venue == 2));
  ui->thesisCombo->setEnabled(new_venue == 5);
}

// Get a citation
void MetaDialog::generateCite()
{
  emit requestCitation(ui->authorsEdit->text(), ui->yearEdit->text());
  emit checkDuplicates(ui->authorsEdit->text(), ui->titleEdit->text(), ui->yearEdit->text());
}

// Request meta to be saved
void MetaDialog::saveAndClose()
{
  PaperMeta meta = GetMeta();
  if(meta.citation.isEmpty()) // TODO or whitespace
  {
    QMessageBox::warning(this, tr("No Citation Set"),
                         tr("Please set a citation before saving"),
                         QMessageBox::Ok);
    return;
  }
  emit updatedDetails(meta);
  accept();
}

// Save but don't close
void MetaDialog::saveOnly()
{
  PaperMeta meta = GetMeta();
  originalCopy = meta;
  emit updatedDetails(meta);
}

// User hit cancel or keyboard shortcut, like escape
void MetaDialog::requestToCancel()
{
  bool will_abandon_edits = false;

  PaperMeta meta = GetMeta();

  if(isEdit)
  {
    if(meta != originalCopy)
      will_abandon_edits = true;
  }
  else
  {
    if(!meta.review.isEmpty() || !meta.authors.isEmpty() || !meta.title.isEmpty()) // May have to expand tests here
      will_abandon_edits = true;
  }

  if(will_abandon_edits)
  {
    // Check with user

    QString close_title(tr("Abandon Review"));
    QString close_message(tr("Are you sure you want to abandon the review?"));

    if(isEdit)
    {
      close_title = tr("Discard Changes");
      close_message = tr("Are you sure you want to discard your edits?");
    }

    int ret = QMessageBox::question(this, close_title,
                                    close_message,
                                    QMessageBox::Ok | QMessageBox::Cancel);

    if(ret == QMessageBox::Ok)
    {
      reject();
      return;
    }
  }
  else
    reject();
}

// User to find the paper PDF or other file type
void MetaDialog::locatePaper()
{
  // Guess paper filename based on citation

  QString cite = ui->citationEdit->text();
  QString guess_stem = paperHuntDir + "/" + cite;

  QString filename = paperHuntDir;

  if(QFile::exists(guess_stem+".doc"))
    filename = guess_stem+".doc";
  else if(QFile::exists(guess_stem+".docx"))
    filename = guess_stem+".docx";
  else if(QFile::exists(guess_stem+".dvi"))
    filename = guess_stem+".dvi";
  else if(QFile::exists(guess_stem+".odt"))
    filename = guess_stem+".odt";
  else if(QFile::exists(guess_stem+".ps"))
    filename = guess_stem+".ps";
  else if(QFile::exists(guess_stem+".ps.gz"))
    filename = guess_stem+".ps.gz";
  else if(QFile::exists(guess_stem+".pdf"))
    filename = guess_stem+".pdf";
  else if(QFile::exists(guess_stem+".txt"))
    filename = guess_stem+".txt";

  locatePaperByName(filename);
}

// User to find the paper PDF or other file type
void MetaDialog::locatePaperByName(const QString &filename)
{
  QString paper_file = QFileDialog::getOpenFileName(this, tr("Locate Paper"), filename,
                                                          tr("Document (*.doc *.docx *.dvi *.odt *.pdf *.ps *.ps.gz *.txt)"));
  if(paper_file.isEmpty()) return;

  ui->paperPathEdit->setText(paper_file);

  checkDenyMove(paper_file);

  emit paperLocated(paper_file);
}

// Add a tag from the tagCombo
// Note: this is essentially OrganiserMain::addTagFilter
void MetaDialog::addTag()
{
  QString current_tag = ui->tagCombo->currentText();
  if(ui->tagsEdit->text().isEmpty())
    ui->tagsEdit->setText(current_tag);
  else
  {
    // Check chosen tag is not present already
    QStringList tag_list = ui->tagsEdit->text().split(",", Qt::SkipEmptyParts);
    bool present = false;
    for(int t = 0; t < tag_list.size(); t++)
    {
      if(tag_list[t] == current_tag)
      {
        present = true;
        break;
      }
    }

    if(!present)
      ui->tagsEdit->setText(ui->tagsEdit->text() + "," + current_tag);
  }
}

// Clear tag list
void MetaDialog::clearTags()
{
  ui->tagsEdit->clear();
}

// Tag combo was activated and there is a new selection
void MetaDialog::tagSelected(int)
{
  addTag();
}

// React to change in citation
void MetaDialog::citationUpdated(const QString &)
{
  bool lacks_citation = ui->citationEdit->text().isEmpty();

  QPushButton *save_button = ui->buttonBox->button(QDialogButtonBox::Save);
  if(save_button) save_button->setEnabled(!lacks_citation);
  saveContinueButton->setEnabled(!lacks_citation);
}

// Check if paper path is already in read papers directory
void MetaDialog::checkDenyMove(const QString &filepath)
{
  if(filepath.startsWith(readPapersPath)) {
    ui->movePapersCheck->setEnabled(false);
  } else {
    ui->movePapersCheck->setEnabled(true);
  }
}

void MetaDialog::keyPressEvent(QKeyEvent *e)
{
  if(e->key() == Qt::Key_Escape)
  {
    requestToCancel();
    e->accept();
  }
  else
    QDialog::keyPressEvent(e);
}
