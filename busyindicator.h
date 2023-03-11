/**
 * @file   busyindicator.h
 * @brief  Widget to show window is busy 
 * @author Lyndon Hill
 * @date   2013.04.21 Incept
 */

#ifndef BUSYINDICATOR_H
#define BUSYINDICATOR_H

#include <QTimer>
#include <QWidget>

/**
 * @class BusyIndicator
 * @brief Draw a small widget that indicates busyness
 */
class BusyIndicator : public QWidget
{
  Q_OBJECT

public:
  /// Constructor
  explicit BusyIndicator(QWidget *parent = 0);

  /// Foreground colour
  void SetColour(QColor colour);

  QSize sizeHint() const;
  QSize minimumSizeHint() const;

public slots:
  /// Start the animation
  void start();

  /// Stop the animation and blank the widget
  void stop();

  /// Update the widget
  void timeIn() { rotate(); }

protected:
  /// Draw the widget
  void paintEvent(QPaintEvent *);

private slots:
  /// Timer calls rotate to change angle and update
  void rotate();

private:
  /**
   * Draw the animation frame
   * @param sideLength  dimension of widget (square)
   * @return the current image
   */
  QPixmap generatePixmap(int sideLength);

  /// Draw the busy ellipse
  void drawEllipse(QPainter *painter);

  /// Animation timer
  QTimer timer;

  /// Angle to start drawing
  int startAngle;

  /// Colour of ellipse
  QColor fillColour;

  /// Count of how many times the indicator has started
  int starts;

  /// Animation is running
  bool running;

  /// Retina mode
  bool isHighDPI;
};

#endif  // BUSYINDICATOR_H
