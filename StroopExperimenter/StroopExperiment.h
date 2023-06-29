/*****************************************************************************
* Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen   *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 27 Jan. 2022                                               *
 *****************************************************************************/
 
#pragma once

#include "Experiment.h"
#include <QTimer>
#include <QColor>
#include <QVector>
#include <QElapsedTimer>


// Scoped enumeration (hence the "struct" keyword) of default type int
// starting at default value 0.
enum struct StroopTrialModes { ColoredQuads, ColoredTextMatched,
                               ColorTextConflicted, ColoredTextUnreferenced };


struct StroopTrial
{
   StroopTrial();
   StroopTrial(StroopTrialModes mode, const QString& text, Qt::GlobalColor color);

   QStringList toStringList(bool includeValidState, bool german) const;
   QString toString(bool includeValidState, bool german) const;

   bool m_bValid;

   StroopTrialModes m_nMode;
   QString m_strText;
   Qt::GlobalColor m_nColor;

   Qt::GlobalColor m_nChosenColor;
   qint64          m_i64DecisionTime;
   bool            m_bCorrect;
};


/**
 * @brief The StroopExperiment class
 */
class StroopExperiment : public Experiment
{
      Q_OBJECT

   public:
      StroopExperiment(int globalIndex, int numTrials, std::weak_ptr<DataReaderWriter> wpDataRW, QObject* parent = nullptr);

      virtual void start();
      virtual void pause();
      virtual void togglePause();
      virtual void stop();

      void onRedChosen();
      void onGreenChosen();
      void onBlueChosen();
      void onYellowChosen();

      QVector<QStringList> currentExperimentSetToString(bool german=false) const;

      virtual QMap<QString, QVariant> getDataToSave();
      virtual void setLoadedData(const QMap<QString, QVariant>& data);

      QStringList getLastStatsStringList() const;

      bool getIndexCreationMode() const;
      void setIndexCreationMode(bool blockOrderMode);

      void activateEvalCorrectTrialsOnlyMode();
      void activateEvalAllTrialsMode();

      bool getEvalCorrectTrialsOnly() const;

      QVector<QStringList> exportLastRunToCSV(const QStringList& headers, bool includeStats) const;
      bool exportAllExperimentsToCSV(const QString& filename, QStringList headers);

   signals:
      void requestFixationPoint();
      void requestColoredQuad(Qt::GlobalColor color);
      void requestColoredWriting(const QString& text, Qt::GlobalColor color);
      void statsComputed(int numMatches, int numWrong, int numTotal,
                         double mean, double stDev );

   private slots:
      void startNextTrial();
      void issueDisplayRequest();

  public slots:
      void storeTimeAndContinue();

   private:
      void createFullyRandomTrialIndices();
      void createEquallyDistributedTrialIndices();
      void createStroopTrials();
      QStringList statsToStringList(double meanRT, int numMatches,
                                    int numWrong, double stDevRT,
                                    bool german=true);
      void checkIfAborted();
      void evaluateTrials();
      void serializeCurrentExperiment();

      int m_nProgress; // It's the "m_nCurrentStroopTrialIndicesIndex" :)
      QVector<int> m_qvecStroopTrialIndices;

      QString     m_strLastExpTimeStamp;
      QStringList m_strlLastStats;

      QElapsedTimer m_eltiSingleDecisionTime;
      QVector<StroopTrial> m_qvecStroopTrials;

      QMap<QString, QVariant> m_mapSerializedResults;

      bool m_bIndexCreationMode;
      bool m_bEvalCorrectTrialsOnly;
      bool m_skipped;

      QTimer timer;
};
