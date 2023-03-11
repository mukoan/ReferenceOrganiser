/**
 * @file   busyindicator.cpp
 * @brief  Widget to show window is busy 
 * @author Lyndon Hill
 * @date   2013.04.21 Incept
 */

#include "busyindicator.h"

#include <QPainter>
#include <QPixmapCache>

// Constructor
BusyIndicator::BusyIndicator(QWidget *parent) : QWidget(parent), startAngle(0), starts(0)
{
  QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  policy.setHeightForWidth(true);
  setSizePolicy(policy);

  fillColour = palette().color(QPalette::WindowText);
  isHighDPI = false;

  running = false;
  timer.setInterval(50);
  connect(&timer, &QTimer::timeout, this, &BusyIndicator::rotate);
}

// Change angle and update
void BusyIndicator::rotate()
{
  startAngle += 30;
  startAngle %= 360;
  update();
}

// Set foreground colour
void BusyIndicator::SetColour(QColor colour)
{
  fillColour = colour;
}

// Start the animation
void BusyIndicator::start()
{
  starts++;
  if(running) return;  // already running

  running = true;
  timer.start();
}

// Stop the animation and blank the widget
void BusyIndicator::stop()
{
  starts--;
  if(starts) return;   // another caller needs busy status

  timer.stop();
  running = false;
  update();
}

// Draw the animation frame
QPixmap BusyIndicator::generatePixmap(int side)
{
  QPixmap pixmap(QSize(side, side));
  pixmap.fill(QColor(255, 255, 255, 0));

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);

  painter.translate(side / 2, side / 2);
  painter.scale(side / 200.0, side / 200.0);
  drawEllipse(&painter);

  return(pixmap);
}

void BusyIndicator::drawEllipse(QPainter *painter)
{
  QColor color = fillColour;
  QBrush brush(color);
  painter->setPen(Qt::NoPen);

  painter->rotate(startAngle);

  float angle = 0;
  while(angle < 360)
  {
    painter->setBrush(brush);
    painter->drawEllipse(-10, -100, 30, 30);

    painter->rotate(30);
    angle += 30;

    color.setAlphaF(angle / 360);
    brush.setColor(color);
  }
}

// Draw the widget
void BusyIndicator::paintEvent(QPaintEvent *)
{
  if(!running) return;

  QPixmap pixmap;
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  if(painter.device()->devicePixelRatio() > 1)
    isHighDPI = true;
  else
    isHighDPI = false;
 
  QString key = QString("%1:%2:%3:%4:%5")
          .arg(metaObject()->className())
          .arg(width())
          .arg(height())
          .arg(startAngle)
          .arg(isHighDPI);

  int side = qMin(width(), height());

  if(!QPixmapCache::find(key, &pixmap))
  {
    if(isHighDPI)
      pixmap = generatePixmap(side*2);
    else
      pixmap = generatePixmap(side);

    QPixmapCache::insert(key, pixmap);
  }

  painter.translate(width() / 2 - side / 2, height() / 2 - side / 2);

  pixmap.setDevicePixelRatio(painter.device()->devicePixelRatio());
  painter.drawPixmap(0, 0, side, side, pixmap);
}

QSize BusyIndicator::minimumSizeHint() const
{
  return(QSize(20, 20));
}

QSize BusyIndicator::sizeHint() const
{
  return(QSize(30, 30));
}
