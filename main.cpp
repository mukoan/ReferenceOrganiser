/**
 * @file   main.cc
 * @brief  Main program
 * @author Lyndon Hill
 * @date   2017.08.01 Incept
 */

#include <QApplication>

#include "organisermain.h"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  OrganiserMain reforg;
  reforg.show();

  return a.exec();
}
