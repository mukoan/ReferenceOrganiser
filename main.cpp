/**
 * @file   main.cc
 * @brief  Main program
 * @author Lyndon Hill
 * @date   2017.08.01 Incept
 */

#include <fstream>
#include <iostream>
#include <string>

#include <QApplication>

#include "organisermain.h"

void getLockFilename(std::string &name)
{
  const char *home = getenv("HOME");
  if(home)
    name = std::string(home) + "/.reference_organiser-lock";
  else
    name = ".reference_organiser-lock";
}

// Check if lockfile exists, create if it doesn't
bool getLock()
{
  std::string lockfile;
  getLockFilename(lockfile);

  std::ifstream test_lock(lockfile);
  if(test_lock.fail())
  {
    // File does not exist - create it
    std::ofstream test_lock;
    test_lock.open(lockfile, std::ios::out);
    test_lock.close();
    return(true);
  }
  else
  {
    std::cerr << "ReferenceOrganiser is already running.\n"
              << "If you are sure it is not running remove " << lockfile << "\n";
    return(false);
  }
}

// Remove lockfile
void releaseLock()
{
  std::string lockfile;
  getLockFilename(lockfile);

  std::remove(lockfile.c_str());
}


int main(int argc, char *argv[])
{
  if(!getLock()) {
    return(EXIT_SUCCESS);
  }

  QApplication a(argc, argv);
  OrganiserMain reforg;
  reforg.show();

  int ret = a.exec();

  releaseLock();
  return ret;
}
