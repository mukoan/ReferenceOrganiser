/**
 * @file   recordlistitem.h
 * @brief  List widget that keeps index of record
 * @author Lyndon Hill
 * @date   2018.02.02
 */

#ifndef RECORDLISTITEM_H
#define RECORDLISTITEM_H

class RecordListItem : public QListWidgetItem
{
public:
  RecordListItem(QListWidget *parent, int index) : QListWidgetItem(parent, QListWidgetItem::Type), recordIndex(index)
  {
  }

  int GetRecordIndex() const { return(recordIndex); }

private:
  int recordIndex;
};

#endif // RECORDLISTITEM_H
