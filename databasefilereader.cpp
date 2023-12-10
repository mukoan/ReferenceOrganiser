/**
 * @file   databasefilereader.cpp
 * @brief  Reference Organiser Database XML reader
 * @author Lyndon Hill
 * @date   2019.04.23
 */

#ifdef DEBUG_XMLREADER
#include <iostream>
#endif

#include "databasefilereader.h"

#include <QDebug>

DatabaseFileReader::DatabaseFileReader() : databaseStarted(false),databaseFileVersion(-1)
{
  targetDatabase = nullptr;
}

// Destructor
DatabaseFileReader::~DatabaseFileReader() { };

// Read the database
bool DatabaseFileReader::Read(QIODevice *device, QString &db_name, QVector<PaperMeta> *database)
{
  targetDatabase = database;

  reader.setDevice(device);

  while(!reader.atEnd())
  {
    reader.readNext();
    if(reader.isStartElement() && (reader.name().toString() == "bibliography"))
    {
      bool result = true;

      if(reader.attributes().hasAttribute("version"))
      {
        databaseFileVersion = reader.attributes().value("version").toInt(&result);
        // if(!result) reader.raiseError(QObject::tr("Database file version not found"));
      }

      if(reader.attributes().hasAttribute("name"))
      {
        db_name = reader.attributes().value("name").toString();
      }

      if(result) readRecordList();
    }
  }

#ifdef DEBUG_XMLREADER
  switch(reader.error())
  {
    case QXmlStreamReader::NoError:                     std::cerr << "Read() no error\n"; break;
    case QXmlStreamReader::NotWellFormedError:          std::cerr << "Read() not well formed\n"; break;
    case QXmlStreamReader::PrematureEndOfDocumentError: std::cerr << "Read() early end of document\n"; break;
    case QXmlStreamReader::UnexpectedElementError:      std::cerr << "Read() unexpected element\n"; break;
    case QXmlStreamReader::CustomError:                 std::cerr << "Read() custom error\n"; break;
  }
#endif

  return(reader.error() == QXmlStreamReader::NoError);
}

// Read the list of records
void DatabaseFileReader::readRecordList()
{
  while(reader.readNextStartElement())
  {
    if(reader.name().toString() == "record")
    {
      record.Clear();

      if(reader.attributes().hasAttribute("citation"))
        record.citation = reader.attributes().value("citation").toString();

      readRecordDetails();
    }
    else
      reader.skipCurrentElement();
  }
}

// Read the record extended metadata
void DatabaseFileReader::readRecordDetails()
{
  // Read the PaperMeta
  readPaperMeta();

  // Store in the database
  targetDatabase->push_back(record);
}

// Read meta data describing the paper
void DatabaseFileReader::readPaperMeta()
{
  while(true)
  {
    QXmlStreamReader::TokenType tt = reader.readNext();
    if(tt == QXmlStreamReader::StartElement)
    {
      QString name = reader.name().toString();
      if(name == "review")
        record.review = reader.readElementText();
      else if(name == "paperPath")
        record.paperPath = reader.readElementText();
      else if(name == "venue")
      {
        QString vts = reader.readElementText();
        if(vts == "Journal")
          record.venue = VenueType::Journal;
        else if(vts == "Conference")
          record.venue = VenueType::Conference;
        else if(vts == "Symposium")
          record.venue = VenueType::Symposium;
        else if(vts == "Book")
          record.venue = VenueType::Book;
        else if(vts == "Preprint")
          record.venue = VenueType::Preprint;
        else if(vts == "Thesis")
          record.venue = VenueType::Thesis;
        else if(vts == "Report")
          record.venue = VenueType::Report;
        else if(vts == "SelfPublished")
          record.venue = VenueType::SelfPublished;
        else if(vts == "NotPublished")
          record.venue = VenueType::NotPublished;
        else if(vts == "None")
          record.venue = VenueType::NoVenue;
        else if(vts == "Unknown")
          record.venue = VenueType::UnknownVenue;
      }
      else if(name == "authors")
        record.authors = reader.readElementText();
      else if(name == "title")
        record.title = reader.readElementText();
      else if(name == "publication")
        record.publication = reader.readElementText();
      else if(name == "volume")
        record.volume = reader.readElementText();
      else if(name == "issue")
        record.issue = reader.readElementText();
      else if(name == "month")
        record.month = reader.readElementText();
      else if(name == "year")
        record.year = reader.readElementText();
      else if(name == "dates")
        record.dates = reader.readElementText();
      else if(name == "pageStart")
        record.pageStart = reader.readElementText();
      else if(name == "pageEnd")
        record.pageEnd = reader.readElementText();
      else if(name == "url")
        record.URL = reader.readElementText();
      else if(name == "thesis")
      {
        QString tts = reader.readElementText();
        if(tts == "Doctorate")
          record.thesis = ThesisType::Doctorate;
        if(tts == "Masters")
          record.thesis = ThesisType::Masters;
        if(tts == "Bachelors")
          record.thesis = ThesisType::Bachelors;
        if(tts == "College")
          record.thesis = ThesisType::College;
        if(tts == "Unknown")
          record.thesis = ThesisType::UnknownThesisType;
      }
      else if(name == "institution")
        record.institution = reader.readElementText();
      else if(name == "location")
        record.location = reader.readElementText();
      else if(name == "publisher")
        record.publisher = reader.readElementText();
      else if(name == "ISBN")
        record.ISBN = reader.readElementText();
      else if(name == "DOI")
        record.doi = reader.readElementText();
      else if(name == "note")
        record.note = reader.readElementText();
      else if(name == "reviewDate")
        record.reviewDate = QDate::fromString(reader.readElementText());
      else if(name == "tags")
        record.tags = reader.readElementText();
      else if(name == "reviewed")
        record.reviewed = true;
      else if(name == "reader")
        readReaderMeta();
      else if(name == "reviewer")
        readReviewerMeta();
    }
    else if((tt == QXmlStreamReader::EndElement) && (reader.name().toString() == "record"))
      break;
  }
}

// Read meta data describing the readers opinion
void DatabaseFileReader::readReaderMeta()
{
  while(true)
  {
    QXmlStreamReader::TokenType tt = reader.readNext();
    if(tt == QXmlStreamReader::StartElement)
    {
      QString name = reader.name().toString();
      if(name == "finished")
        record.reader.finished = true;
      else if(name == "understanding")
        record.reader.understanding = reader.readElementText().toInt();
      else if(name == "rating")
        record.reader.rating = reader.readElementText().toInt();
    }
    else if((tt == QXmlStreamReader::EndElement) && (reader.name().toString() == "reader"))
      return;
  }
}

// Read meta data describing the reviewers opinion
void DatabaseFileReader::readReviewerMeta()
{
  while(true)
  {
    QXmlStreamReader::TokenType tt = reader.readNext();
    if(tt == QXmlStreamReader::StartElement)
    {
      QString name = reader.name().toString();
      if(name == "accept")
      {
        QString accept_type = reader.readElementText();
        if(accept_type == "AcceptStrong") record.reviewer.accept = Accept_Strong;
        else if(accept_type == "AcceptWeak") record.reviewer.accept = Accept_Weak;
        else if(accept_type == "AcceptNeutral") record.reviewer.accept = Accept_Neutral;
        else if(accept_type == "RejectWeak") record.reviewer.accept = Reject_Weak;
        else if(accept_type == "RejectStrong") record.reviewer.accept = Reject_Strong;
      }
      else if(name == "suitability")
        record.reviewer.suitability = reader.readElementText().toInt();
      else if(name == "correctness")
        record.reviewer.technicalCorrectness = reader.readElementText().toInt();
      else if(name == "novelty")
        record.reviewer.novelty = reader.readElementText().toInt();
      else if(name == "clarity")
        record.reviewer.clarity = reader.readElementText().toInt();
      else if(name == "relevance")
        record.reviewer.relevance = reader.readElementText().toInt();
      else if(name == "corrections")
        record.reviewer.correctionsRequired = true;
      else if(name == "authorComments")
        record.reviewer.commentsToAuthors = reader.readElementText();
      else if(name == "editorComments")
        record.reviewer.commentsToChairEditor = reader.readElementText();
    }
    else if((tt == QXmlStreamReader::EndElement) && (reader.name().toString() == "reviewer"))
      return;
  }
}
