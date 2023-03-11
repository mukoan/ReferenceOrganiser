/**
 * @file   databasefilewriter.cpp
 * @brief  Reference Organiser Database XML writer
 * @author Lyndon Hill
 * @date   2021.01.09
 */

#include <QFile>
#include <QXmlStreamWriter>

#include "databasefilewriter.h"

// Constructor
DatabaseFileWriter::DatabaseFileWriter()
{
  version = DATABASE_VERSION;
}

// Save a paper database
bool DatabaseFileWriter::Save(const QString &filename, const QString &name, const QVector<PaperMeta> &records)
{
  QFile output(filename);
  if(!output.open(QIODevice::WriteOnly | QIODevice::Text))
    return(false);

  QXmlStreamWriter stream(&output);
  stream.setAutoFormatting(true);
  stream.writeStartDocument();

  stream.writeStartElement("bibliography");
  stream.writeAttribute("version", version);
  stream.writeAttribute("name", name);          // Write database name

  // Write metadata for all papers
  for(int r = 0; r < records.size(); r++)
  {
    stream.writeStartElement("record");
    stream.writeAttribute("citation", records[r].citation);

    if(!records[r].paperPath.isEmpty())
      stream.writeTextElement("paperPath", records[r].paperPath);

    if(!records[r].review.isEmpty())
      stream.writeTextElement("review", records[r].review);

    switch(records[r].venue)
    {
      case VenueType::Journal:       stream.writeTextElement("venue", "Journal");       break;
      case VenueType::Conference:    stream.writeTextElement("venue", "Conference");    break;
      case VenueType::Symposium:     stream.writeTextElement("venue", "Symposium");     break;
      case VenueType::Book:          stream.writeTextElement("venue", "Book");          break;
      case VenueType::Preprint:      stream.writeTextElement("venue", "Preprint");      break;
      case VenueType::Thesis:        stream.writeTextElement("venue", "Thesis");        break;
      case VenueType::Report:        stream.writeTextElement("venue", "Report");        break;
      case VenueType::SelfPublished: stream.writeTextElement("venue", "SelfPublished"); break;
      case VenueType::NotPublished:  stream.writeTextElement("venue", "NotPublished");  break;
      case VenueType::NoVenue:       stream.writeTextElement("venue", "None");          break;
      case VenueType::UnknownVenue:  stream.writeTextElement("venue", "Unknown");       break;
    }

    if(!records[r].authors.isEmpty())
      stream.writeTextElement("authors", records[r].authors);
    if(!records[r].title.isEmpty())
      stream.writeTextElement("title", records[r].title);
    if(!records[r].publication.isEmpty())
      stream.writeTextElement("publication", records[r].publication);

    if(!records[r].volume.isEmpty())
      stream.writeTextElement("volume", records[r].volume);
    if(!records[r].issue.isEmpty())
      stream.writeTextElement("issue", records[r].issue);
    if(!records[r].month.isEmpty())
      stream.writeTextElement("month", records[r].month);
    if(!records[r].year.isEmpty())
      stream.writeTextElement("year", records[r].year);
    if(!records[r].dates.isEmpty())
      stream.writeTextElement("dates", records[r].dates);

    if(!records[r].pageStart.isEmpty())
      stream.writeTextElement("pageStart", records[r].pageStart);
    if(!records[r].pageEnd.isEmpty())
      stream.writeTextElement("pageEnd", records[r].pageEnd);

    if(!records[r].URL.isEmpty())
      stream.writeTextElement("url", records[r].URL);

    if(records[r].venue == VenueType::Thesis)
    {
      switch(records[r].thesis)
      {
        case ThesisType::Doctorate:         stream.writeTextElement("thesis", "Doctorate"); break;
        case ThesisType::Masters:           stream.writeTextElement("thesis", "Masters");   break;
        case ThesisType::Bachelors:         stream.writeTextElement("thesis", "Bachelors"); break;
        case ThesisType::College:           stream.writeTextElement("thesis", "College");   break;
        case ThesisType::UnknownThesisType: stream.writeTextElement("thesis", "Unknown");   break;
      }
    }

    if(!records[r].institution.isEmpty())
      stream.writeTextElement("institution", records[r].institution);

    if(!records[r].location.isEmpty())
      stream.writeTextElement("location", records[r].location);

    if(!records[r].publisher.isEmpty())
      stream.writeTextElement("publisher", records[r].publisher);

    if(!records[r].ISBN.isEmpty())
      stream.writeTextElement("ISBN", records[r].ISBN);

    if(!records[r].doi.isEmpty())
      stream.writeTextElement("DOI", records[r].doi);

    if(!records[r].note.isEmpty())
      stream.writeTextElement("note", records[r].note);

    if(records[r].reviewDate.isValid())
      stream.writeTextElement("reviewDate", records[r].reviewDate.toString());

    if(!records[r].tags.isEmpty())
      stream.writeTextElement("tags", records[r].tags);

    // Reader opinion
    if((records[r].reader.finished) || (records[r].reader.understanding > 1) || (records[r].reader.rating > 1))
    {
      stream.writeStartElement("reader");
      if(records[r].reader.finished)
        stream.writeEmptyElement("finished");

      stream.writeTextElement("understanding", QString::number(records[r].reader.understanding));
      stream.writeTextElement("rating", QString::number(records[r].reader.rating));

      stream.writeEndElement(); // reader
    }

    // Review
    if((records[r].reviewer.accept != Accept_Neutral) || (records[r].reviewer.correctionsRequired) ||
       (records[r].reviewer.suitability > 1) || (records[r].reviewer.technicalCorrectness > 1) ||
       (records[r].reviewer.novelty > 1) || (records[r].reviewer.clarity > 1) || (records[r].reviewer.relevance > 1) ||
       (!records[r].reviewer.commentsToAuthors.isEmpty()) || (!records[r].reviewer.commentsToChairEditor.isEmpty()))
    {
      stream.writeStartElement("reviewer");

      switch(records[r].reviewer.accept)
      {
        case Accept_Strong:
        stream.writeTextElement("accept", "AcceptStrong");
        break;

        case Accept_Weak:
        stream.writeTextElement("accept", "AcceptWeak");
        break;

        case Accept_Neutral:
        stream.writeTextElement("accept", "AcceptNeutral");
        break;

        case Reject_Weak:
        stream.writeTextElement("accept", "RejectWeak");
        break;

        case Reject_Strong:
        stream.writeTextElement("accept", "RejectStrong");
        break;


      }

      stream.writeTextElement("suitability", QString::number(records[r].reviewer.suitability));
      stream.writeTextElement("correctness", QString::number(records[r].reviewer.technicalCorrectness));
      stream.writeTextElement("novelty",     QString::number(records[r].reviewer.novelty));
      stream.writeTextElement("clarity",     QString::number(records[r].reviewer.clarity));
      stream.writeTextElement("relevance",   QString::number(records[r].reviewer.relevance));

      if(records[r].reviewer.correctionsRequired)
        stream.writeEmptyElement("corrections");

      if(!records[r].reviewer.commentsToAuthors.isEmpty())
        stream.writeTextElement("authorComments", records[r].reviewer.commentsToAuthors);

      if(!records[r].reviewer.commentsToChairEditor.isEmpty())
        stream.writeTextElement("editorComments", records[r].reviewer.commentsToChairEditor);

      stream.writeEndElement(); // reviewer
    }

    // all data is stored in review...
    stream.writeEndElement(); // record
  }

  stream.writeEndElement(); // bibliography
  stream.writeEndDocument();

  output.close();

  return(true);
}
