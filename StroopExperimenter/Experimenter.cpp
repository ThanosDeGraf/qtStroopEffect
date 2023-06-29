/*****************************************************************************
* Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen   *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 1 Apr. 2022                                                *
 *****************************************************************************/
 
#include "Experimenter.h"
#include "StroopExperiment.h"
#include "DataReaderWriter.h"

#include <QRegularExpression>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDir>

#include <iostream>


/**
 * @brief Experimenter::Experimenter
 * @param parent
 */
Experimenter::Experimenter(std::weak_ptr<DataReaderWriter> dataRW, QObject* parent)
   : QObject(parent)
   , m_wpDataRW(dataRW)
   , m_nNumExperimentRuns(100)
{
   QString personID("default");
   QString expName("stroop");
   QString filePath(personID + "." + expName);
   QStringList fileInfo;
   fileInfo << personID << expName << filePath;

   m_pairLastLoadedExperimentInfo =
         QPair<bool,QStringList>(false, fileInfo);
}


/**
 * @brief Experimenter::getExperimentNamesList
 * @return
 */
QStringList Experimenter::getExperimentNamesList() const
{
   return m_strliExperimentNames;
}


/**
 * @brief Experimenter::setExperimentNamesList
 * @param qvecExperiments
 */
void Experimenter::setExperimentNamesList(const QStringList& experimentNames)
{
   m_strliExperimentNames = experimentNames;

   createExperiments();
}


/**
 * @brief Experimenter::createExperiments
 */
void Experimenter::createExperiments()
{
   int numExp = m_strliExperimentNames.count();

   for (int idx=0; idx<numExp; idx++)
   {
      const QString& key = m_strliExperimentNames.at(idx);

      if (key == "stroop")
      {
         std::shared_ptr<StroopExperiment> exp =
               std::make_shared<StroopExperiment>(idx, m_nNumExperimentRuns,
                                                  m_wpDataRW);

         connect(exp.get(), &StroopExperiment::started,
                 this, &Experimenter::experimentStarted);
         connect(exp.get(), &StroopExperiment::stopped,
                 this, &Experimenter::experimentStopped);   // (re-)send signal
         connect(exp.get(), &StroopExperiment::stopped,
                 this, &Experimenter::onExperimentStopped); // slot

         m_qmapExperiments.insert(key, exp);
      }
   }
}


/**
 * @brief parseFileName
 * @param fileLocation May contain the whole path or just the filename
 * @return QStringList with [0]:PersonID - [1]:ExperimentName - [2]:FullfileLocation
 */
QStringList Experimenter::parseFileName(const QString& fileLocation)
{
   QStringList result;

   // Handle the two cases of full path or just file name
   QFileInfo fileInfo(fileLocation);
   const QString& fileName = fileInfo.fileName();
   QStringList fileNameParts = fileName.split(".");

   // The filename should read <person's name>.<experiment type>
   if (fileNameParts.count() == 2)
   {
      result.append(fileNameParts.first());
      result.append(fileNameParts.last().toLower().replace(" ",""));

      QString filePath = fileInfo.filePath();

      // If fileInfo only contains the name, assume the file is where the executable is.
      if (filePath == fileName)
      {
         QDir appPath(QCoreApplication::applicationDirPath());
         filePath = appPath.absoluteFilePath(fileName);
      }      

      result.append(filePath);
   }

   return result;
}


/**
 * @brief Experimenter::loadExperiment
 * @param fileName
 * @return
 */
bool Experimenter::loadExperiment(const QString& fileName)
{
   // Get info about the experiment to load
   QStringList fileInfo = parseFileName(fileName);
   if (fileInfo.isEmpty()) {
      // "File name wrongly formatted (couldn't be parsed)."
      return false;
   }

   const QString& personID = fileInfo.at(0);
   const QString& expName = fileInfo.at(1);
   const QString& filePath = fileInfo.at(2);

   // Check if the experiment is supported
   bool expSupported = m_strliExperimentNames.contains(expName, Qt::CaseInsensitive);
   if (!expSupported) {
      // QString("The specified type of experiment ('%1') is not supported.").arg(expName);
      return false;
   }

   // Update matching Experiment instance and set it as active
   std::shared_ptr<Experiment> exp = m_qmapExperiments.value(expName);
   if (exp)
   {
      exp->setExperimentName(expName);
      exp->setPersonID(personID);
   }

   // Create empty file if the specified one doesn't exist.
   if ( !QFileInfo::exists(filePath) )
   {
      QFile file(filePath);
      file.open(QIODevice::ReadWrite | QIODevice::Text);
      file.close();

      std::cout << "Created specified file: " << filePath.toStdString() << std::endl;
   }
   else
   {
      // Load data (for running statistics or visualization...)
      if (std::shared_ptr<DataReaderWriter> spDataRW = m_wpDataRW.lock())
      {
         QMap<QString, QVariant> data;
         spDataRW->loadData(filePath, data);
         exp->setLoadedData(data);
      }
   }
   // Save info about currently loaded file.
   m_pairLastLoadedExperimentInfo = QPair<bool,QStringList>(true, fileInfo);

   // Signal that a new Experiment has been loaded and activated
   QRegularExpression regExp(expName, QRegularExpression::CaseInsensitiveOption);
   int expID = m_strliExperimentNames.indexOf(regExp);
   emit experimentLoaded(expID);

   return true;
}


/**
 * @brief Experimenter::getLastLoadedExperimentInfo
 * @return
 */
QPair<bool, QStringList> Experimenter::getLastLoadedExperimentInfo()
{
   return m_pairLastLoadedExperimentInfo;
}


/**
 * @brief Experimenter::saveExperiment
 * @param fileName
 */
void Experimenter::saveExperiment(const QString& fileName, const QString& expName)
{
   if (std::shared_ptr<DataReaderWriter> spDataRW = m_wpDataRW.lock())
   {
      std::shared_ptr<Experiment> exp = m_qmapExperiments.value(expName);

      QMap<QString, QVariant> data = exp->getDataToSave();
      spDataRW->saveData(fileName, data);
   }
}


/**
 * @brief Experimenter::exportCSV
 * @param stringData
 */
void Experimenter::exportCSV(const QString& fileName, QVector<QStringList> stringData)
{
   if (std::shared_ptr<DataReaderWriter> spDataRW = m_wpDataRW.lock())
   {
      spDataRW->writeCSV(fileName, stringData);
   }
}


/**
 * @brief Experimenter::onTabChanged
 */
void Experimenter::onTabChanged(int tabID)
{
   Q_UNUSED(tabID);
}


/**
 * @brief Experimenter::saveCurrentFile
 */
void Experimenter::onExperimentStopped(int idx)
{
   Q_UNUSED(idx); // Use idx to double check signal sender if needed.

   const QString& expName = m_pairLastLoadedExperimentInfo.second.at(1);
   const QString& filePath = m_pairLastLoadedExperimentInfo.second.at(2);

   saveExperiment(filePath, expName);
}


/**
 * @brief Experimenter::addExperiment
 * @param name
 * @param experiment
 */
void Experimenter::addExperiment(const QString& name,
                                 std::shared_ptr<Experiment> experiment)
{
   m_qmapExperiments.insert(name.toLower(), experiment);
}


/**
 * @brief Experimenter::getExperiment
 * @param name
 * @return
 */
std::shared_ptr<Experiment> Experimenter::getExperiment(const QString& expName) const
{
   return m_qmapExperiments.value(expName);
}


/**
 * @brief Experimenter::removeExperiment
 * @param name
 */
void Experimenter::removeExperiment(const QString& name)
{
   m_qmapExperiments.remove(name.toLower());
}


/**
 * @brief Experimenter::setNumExperimentRuns
 * @param numExperimentRuns
 */
void Experimenter::setNumTrials(int numExperimentRuns)
{
   m_nNumExperimentRuns = numExperimentRuns;

   // Iterating all elements of a QMap without allocating a tempory container
   QMap< QString, std::shared_ptr<Experiment> >::key_iterator it;
   for (it = m_qmapExperiments.keyBegin(); it != m_qmapExperiments.keyEnd(); it++)
   {
      std::shared_ptr<Experiment> exp = m_qmapExperiments.value(*it);
      exp->setNumTrials(m_nNumExperimentRuns);
   }
}
