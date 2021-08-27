/**
 * @file   paperrecord.h
 * @brief  Basic record for a paper
 * @author Lyndon Hill
 * @date   2017.08.01
 */

#ifndef PAPERRECORD_H
#define PAPERRECORD_H

#include <QString>

/// Basic record of a paper
class PaperRecord
{
public:
  QString citation;    ///< Citation key
  QString reviewPath;  ///< Path to the review
  QString paperPath;   ///< Path to the paper

  void Clear()
  {
    citation.clear();
    reviewPath.clear();
    paperPath.clear();
  }

  /// Comparison for sorting
  bool operator<(const PaperRecord &rhs) const
  {
    return(citation < rhs.citation);
  }
};

#endif  // PAPERRECORD_H
