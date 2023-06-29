/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 16 Dec. 2021                                               *
 *****************************************************************************/
 
#pragma once

#include <QObject>
#include <QMap>
#include <QStringList>

// Forward declarations
class DataReaderWriter;
class Experiment;


/**
 * @brief The Experimenter class
 */
class Experimenter : public QObject
{
   Q_OBJECT

   public:
      explicit Experimenter(std::weak_ptr<DataReaderWriter> dataRW, QObject* parent = nullptr);

      bool loadExperiment(const QString& fileName);
      QPair<bool, QStringList> getLastLoadedExperimentInfo();
      void saveExperiment(const QString& fileName, const QString& expName);

      void exportCSV(const QString& fileName, QVector<QStringList> stringData);

      QStringList getExperimentNamesList() const;
      void setExperimentNamesList(const QStringList& experiments);

      std::shared_ptr<Experiment> getExperiment(const QString& expName) const;


   public slots:
      void onTabChanged(int tabID);
      void setNumTrials(int numExperimentRuns);


   signals:
      void experimentLoaded(int idx);
      void experimentStarted(int idx);
      void experimentStopped(int idx);

   private slots:
      void onExperimentStopped(int idx);

   private:
      // Methods
      void createExperiments();

      void addExperiment(const QString& name,
                         std::shared_ptr<Experiment> experiment);
      void removeExperiment(const QString& name);

      QStringList parseFileName(const QString& filePath);

      // Variables
      QStringList m_strliExperimentNames;

      QMap< QString, std::shared_ptr<Experiment> > m_qmapExperiments;

      std::weak_ptr<DataReaderWriter> m_wpDataRW;

      // loaded, person ID, experiment name, file path
      QPair<bool, QStringList> m_pairLastLoadedExperimentInfo;

      int m_nNumExperimentRuns;
};
