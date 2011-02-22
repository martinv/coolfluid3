// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>

#include <QApplication>

#include "Common/CF.hpp"
#include "Common/Core.hpp"
#include "Common/Exception.hpp"

#include "GUI/Client/Core/ClientRoot.hpp"
#include "GUI/Client/Core/NCore.hpp"
#include "GUI/Client/UI/MainWindow.hpp"
#include "GUI/Client/UI/JournalBrowserDialog.hpp"

using namespace CF::Common;
using namespace CF::GUI::ClientCore;
using namespace CF::GUI::ClientUI;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  int returnValue;

//  JournalBrowserBuilder::instance();

  CF::AssertionManager::instance().AssertionThrows = true;
  CF::AssertionManager::instance().AssertionDumps = true;

  // tell CF core that the client is running
  Core::instance().network_info().start_client();

  try
  {
   MainWindow window;
   window.showMaximized();
//   TSshInformation info("localhost", 62784);

//   NCore::globalCore()->connectToServer(info);
   returnValue = app.exec();
  }
  catch(Exception e)
  {
    std::cerr << "Application stopped on uncaught exception:" << std::endl;
    std::cerr << e.what() << std::endl;
    returnValue = -1;
  }

  // tell CF core that the client is about to exit
  Core::instance().network_info().stop_client();


  return returnValue;
}
