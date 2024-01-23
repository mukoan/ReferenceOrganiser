/**
 * @file   history.cpp
 * @brief  Store history
 * @author Lyndon Hill
 * @date   2023.10.19
 */

#include "history.h"

#include <QObject>

// Constructor
History::History(int max_size)
{
  items = QList<HistoryItem>();
  maximumSize = max_size;
}

// Something happened
void History::ReportAction(const HistoryItem &hitem)
{
  // Check hitem is not a duplicate of the last action
  if(!items.isEmpty())
  {
    HistoryItem &first = items.first();
    if((first.text == hitem.text) && (first.action == hitem.action)) return;
  }

  // If hitem.text is in the list then remove and add to front

  bool in_list = false;

  QList<HistoryItem>::const_iterator i = items.constBegin();
  while(i != items.constEnd())
  {
    if(i->text == hitem.text)
    {
      items.erase(i);
      in_list = true;
      break;
    }
    ++i;
  }

  if(!in_list)
  {
    if(items.count() >= maximumSize)
      items.pop_back();
  }

  items.push_front(hitem);
}

// Reports an action by creating a HistoryItem; as function above
void History::ReportAction(const QString &str, const ROAction &act)
{
  HistoryItem hit;
  hit.text   = str;
  hit.action = act;

  ReportAction(hit);
}

// Most recent thing to have happened
HistoryItem History::LastAction()
{
  if(items.count() >= 1)
    return(items.front());
  else
  {
    HistoryItem it;
    it.text   = QObject::tr("(empty)");
    it.action = ROAction::None;
    return(it);
  }
}

// Number of items stored
unsigned int History::Count()
{
  return(items.count());
}

// Access items
HistoryItem History::operator[](int i)
{
  if(i >= items.count())
  {
    HistoryItem it;
    it.text   = QObject::tr("(empty)");
    it.action = ROAction::None;
    return(it);
  }
  else
    return(items[i]);
}
