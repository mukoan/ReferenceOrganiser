/**
 * @file   reviewparser.h
 * @brief  Load and parse a review
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#ifndef REVIEWPARSER_H
#define REVIEWPARSER_H

#include <QString>
#include <QStringList>

/**
 * @brief  Load a review
 * @param  filename  full path to the review
 * @param  review    text of the review
 * @return success
 */
bool LoadReview(const QString &filename, QString &review);

/// Get embedded details from the review
void ParseReview(const QString &filename, const QString &review, QString &authors, QString &title, QString &year);

/**
 * Find the year appended at the end of the string
 * @param cite   citation
 * @param year   output (if found)
 * @return true if a year was found
 */
bool ParseYearFromCitation(const QString &cite, QString &year);

/// Trim title and authors line from review
void TrimReviewHeader(QString &review);

/**
 * @brief  Parse authors
 * @param  authors_csv  comma separated list of authors
 * @return a list of individual authors
 */
QStringList ParseAuthors(const QString &authors_csv);

#endif // REVIEWPARSER_H
