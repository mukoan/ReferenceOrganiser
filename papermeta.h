/**
 * @file   papermeta.h
 * @brief  Paper metadata
 * @author Lyndon Hill
 * @date   2019.03.03
 */

#ifndef PAPERMETA_H
#define PAPERMETA_H

#include <QString>
#include <QDate>

/// Type of place where a paper was published
enum class VenueType
{
  Journal,
  Conference,
  Symposium,
  Book,
  Preprint,
  Thesis,
  Report,
  SelfPublished,
  NotPublished,
  NoVenue,
  UnknownVenue
};

/// Type of thesis that reference might be
enum class ThesisType
{
  Doctorate,
  Masters,
  Bachelors,
  College,
  UnknownThesisType
};

enum Acceptance
{
  Accept_Strong  = 0,
  Accept_Weak    = 1,
  Accept_Neutral = 2,
  Reject_Weak    = 3,
  Reject_Strong  = 4
};

/// Reader's thoughts on the paper (summary)
class ReaderMeta
{
public:
  ReaderMeta() : finished(false),understanding(1),rating(1) {}

  bool        finished;
  int         understanding;           ///< 1-10, understood = 10
  int         rating;                  ///< 1-10, uninteresting = 1

  /// Equality function
  bool operator==(const ReaderMeta &rhs) const
  {
    if( (finished == rhs.finished) && (understanding == rhs.understanding) &&
        (rating == rhs.rating) )
      return(true);

    return(false);
  }
};

/// Reviewer's thoughts on the paper (peer review)
class ReviewerMeta
{
public:

  ReviewerMeta()
  {
    accept               = Accept_Neutral;
    suitability          = 1;
    technicalCorrectness = 1;
    novelty              = 1;
    clarity              = 1;
    relevance            = 1;
    correctionsRequired  = false;
  }

  Acceptance  accept;                  ///< The paper should be accepted
  int         suitability;             ///< The paper's suitability to the venue
  int         technicalCorrectness;    ///< The correctness of the paper
  int         novelty;                 ///< Novelty/inventiveness
  int         clarity;                 ///< Quality of writing and images
  int         relevance;               ///< The subject matter is relevant to the reviewer's background
  bool        correctionsRequired;     ///< Paper needs changes before publication
  QString     commentsToAuthors;       ///< Comments sent back to authors
  QString     commentsToChairEditor;   ///< Comments sent back to editor

  /// Equality function
  bool operator==(const ReviewerMeta &rhs) const
  {
    if( (accept == rhs.accept) && (suitability == rhs.suitability) &&
        (technicalCorrectness == rhs.technicalCorrectness) && (novelty == rhs.novelty) &&
        (clarity == rhs.clarity) && (relevance == rhs.relevance) && (correctionsRequired == rhs.correctionsRequired) &&
        (commentsToAuthors == rhs.commentsToAuthors) && (commentsToChairEditor == rhs.commentsToChairEditor) )
      return(true);

    return(false);
  }
};

/// Details for someone to find the paper again
class PaperMeta
{
public:
  /// Constructor
  PaperMeta() : year("-1")
  {
    venue  = VenueType::UnknownVenue;
    thesis = ThesisType::UnknownThesisType;

    reviewed             = false;
    pseudo               = false;
    ingest               = false;

    reader.finished      = false;
    reader.understanding = 1;
    reader.rating        = 1;
  }

  QString     citation;    ///< Citation key
  QString     review;      ///< Review text
  QString     paperPath;   ///< Path to the paper

  VenueType   venue;       ///< How the paper was published
  QString     authors;     ///< Who wrote the paper
  QString     title;       ///< Paper title
  QString     publication; ///< The name of the venue
  QString     volume;      ///< Volume number
  QString     issue;       ///< Issue number
  QString     month;       ///< Month (as a number)
  QString     year;        ///< Year (as a 4 digit number)
  QString     dates;       ///< Range of dates of conference
  QString     pageStart;   ///< First page of paper
  QString     pageEnd;     ///< Last page of paper
  QString     URL;         ///< Link to a page with the paper, pref author's page
  ThesisType  thesis;      ///< Type of thesis
  QString     institution; ///< University or Company if only one for all authors
  QString     location;    ///< Location of the conference/symposium
  QString     publisher;   ///< Name of company printing
  QString     ISBN;        ///< Internation Standard Book Number
  QString     doi;         ///< Digital Object Identifier, in form of "10.1000/182"
  QString     note;        ///< Reference note
  QDate       reviewDate;  ///< Date review entered
  QString     tags;        ///< tags for user to taxonomise papers; comma separated
  bool        reviewed;    ///< Paper has been reviewed for publication, see reviewer member

  bool        pseudo;      ///< This is a temporary meta for a paper without a review
  bool        ingest;      ///< The paper should be moved to the read papers directory (not saved in database)

  QString     originalCitation;  ///< If the citation is changed; this stores the original citation

  ReaderMeta    reader;    ///< For readers to rank papers
  ReviewerMeta  reviewer;  ///< For paper reviewers

  /// Clear the data
  void Clear()
  {
    citation.clear(); review.clear();     paperPath.clear();
    venue = VenueType::UnknownVenue;      authors.clear(); title.clear();
    publication.clear(); volume.clear();  issue.clear();
    month.clear();       year.clear();    dates.clear();
    pageStart.clear();   pageEnd.clear(); URL.clear();
    thesis = ThesisType::UnknownThesisType; institution.clear(); location.clear();
    publisher.clear();   ISBN.clear();    doi.clear(); note.clear();
    reviewDate = QDate();                 tags.clear();
    reviewed = false;
    pseudo = false;
    ingest = false;

    reader.finished      = false;
    reader.understanding = 1;
    reader.rating        = 1;

    reviewer.accept = Accept_Neutral;
    reviewer.suitability = 1;
    reviewer.technicalCorrectness = 1;
    reviewer.novelty = 1;
    reviewer.clarity = 1;
    reviewer.relevance = 1;
    reviewer.correctionsRequired = false;
    reviewer.commentsToAuthors.clear();
    reviewer.commentsToChairEditor.clear();
  }


  /// Comparison for sorting
  bool operator<(const PaperMeta &rhs) const
  {
    return(citation < rhs.citation);
  }

  /// Equality function
  bool operator==(const PaperMeta &rhs) const
  {
    if( (citation == rhs.citation) && (review == rhs.review) && (paperPath == rhs.paperPath) &&
        (venue == rhs.venue) && (authors == rhs.authors) && (title == rhs.title) &&
        (publication == rhs.publication) && (volume == rhs.volume) && (issue == rhs.issue) &&
        (month == rhs.month) && (year == rhs.year) && (dates == rhs.dates) &&
        (pageStart == rhs.pageStart) && (pageEnd == rhs.pageEnd) && (URL == rhs.URL) &&
        (thesis == rhs.thesis) && (institution == rhs.institution) && (location == rhs.location) &&
        (publisher == rhs.publisher) && (ISBN == rhs.ISBN) && (doi == rhs.doi) &&
        (note == rhs.note) && (reviewDate == rhs.reviewDate) && (tags == rhs.tags) &&
        (reviewed == rhs.reviewed) && (reader == rhs.reader) && (reviewer == rhs.reviewer) )
    {
      return(true);
    }
    else
      return(false);
  }

  /// Inequality function
  bool operator!=(const PaperMeta &rhs) const
  {
    return(!(*this == rhs));
  }
};

#endif  // PAPERMETA_H
