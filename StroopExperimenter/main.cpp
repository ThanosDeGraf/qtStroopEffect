/*****************************************************************************
* Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen   *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 20 Jan. 2022                                               *
 *****************************************************************************/
 
#include "DataReaderWriter.h"
#include "Experimenter.h"
#include "MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <iostream>


/**
 * @brief main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[])
{
   QApplication app(argc, argv);
   QApplication::setApplicationName("Stroop Experimenter");
   QApplication::setApplicationVersion("1.0");

   // 3-tier application design: 1. data, 2. logic, 3. GUI
   // Here, "Dependency Injection" is implemented by using "Constructor Injection"
   std::shared_ptr<DataReaderWriter> spDataRW = std::make_shared<DataReaderWriter>();
   std::shared_ptr<Experimenter> spExperimenter = std::make_shared<Experimenter>(spDataRW);
   std::shared_ptr<MainWindow> spMainWindow = std::make_shared<MainWindow>(spExperimenter);

   // Configure command line parser
   QCommandLineParser parser;
   parser.setApplicationDescription("Performs several runs of the Stroop experiment.");
   parser.addHelpOption();
   parser.addVersionOption();

   QCommandLineOption numTrialsOption("n", "Number of experiment runs.");
   parser.addOption(numTrialsOption);

   QCommandLineOption folderOption("o", "<folder> - <folder> in which the *.stroop file is located.");
   parser.addOption(folderOption);

   QCommandLineOption fileOption("f", "<name>.stroop file - <name> is used to identify the participant.");
   parser.addOption(fileOption);

   // Process the given command line arguments
   parser.process(app);

   if (parser.isSet(numTrialsOption))
   {
      QString numRunStr = parser.value(numTrialsOption);
      if (numRunStr.isEmpty())
      {
         numRunStr = parser.positionalArguments().at(parser.optionNames().indexOf("n"));
      }

      bool converted;
      int numTrials = numRunStr.toInt(&converted);
      if (!converted)
      {
         numTrials = 100;
      }

      spMainWindow->setNumExperimentRuns(numTrials);
   }
   else
   {
      spMainWindow->setNumExperimentRuns(12); // 100
   }

   // Specifiy folder for .stroop file
   QString folderPath;
   if (parser.isSet(folderOption))
   {
      folderPath = parser.value(folderOption);
      if (folderPath.isEmpty())
      {
         folderPath = parser.positionalArguments().at(parser.optionNames().indexOf(*folderOption.names().constBegin()));
      }
   }

   // Specifiy .stroop file
   if (parser.isSet(fileOption))
   {
      QString fileName = parser.value(fileOption);
      if (fileName.isEmpty())
      {
         fileName = parser.positionalArguments().at(parser.optionNames().indexOf(*fileOption.names().constBegin()));
      }

      if (!folderPath.isEmpty())
      {
         QStringList fileNameList = fileName.split(QDir::separator());
         fileName = folderPath + QDir::separator() + fileNameList.last();
      }

      spExperimenter->loadExperiment(fileName);
   }
   else
   {
      spExperimenter->loadExperiment("default.stroop");
   }

   spMainWindow->show();

   return app.exec();
}
