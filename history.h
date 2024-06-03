/**
 * @file   history.h
 * @brief  Store history
 * @author Lyndon Hill
 * @date   2023.10.19
 */

#ifndef HISTORY_H
#define HISTORY_H

#include <QString>
#include <QList>

/// Things you can do in ReferenceOrganiser
enum class ROAction
{
  View,              ///< View a review
  Edit,              ///< Edit a review
  Create,            ///< Create a review
  Add,               ///< Add a paper
  None               ///< Not an action 
};

/// Something you did
struct HistoryItem
{
  QString  text;     ///< Description/citation/path
  ROAction action;   ///< The action
};

/// Handle all history
class History
{
public:
  /// Constructor
  History(int max_size = 50);

  /// Destructor
  ~History() {};

  /// Something happened
  void ReportAction(const HistoryItem &hitem);

  /**
   * @brief ReportAction - reports an action by creating a HistoryItem
   * @param str  text for the thing you did
   * @param act  the action it was
   */
  void ReportAction(const QString &str, const ROAction &act);

  /// Most recent thing to have happened
  HistoryItem LastAction();

  /// Number of items stored
  unsigned int Count();

  /// Access items
  HistoryItem operator[](int i);

  /**
   * Citation had name changed
   * @return true if citation was updated or false if not found
   */
  bool RenameCitation(const QString &original, const QString &updated);

private:
  /// The list of items
  QList<HistoryItem> items;

  /// Maximum size of the list
  unsigned int maximumSize;
};

#endif // HISTORY_H
