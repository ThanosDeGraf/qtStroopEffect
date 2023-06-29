/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 15 Dec. 2021                                               *
 *****************************************************************************/
 
#pragma once

#include <QObject>
#include <QMap>
#include <QVariant>
#include "DataReaderWriter.h"


/**
 * @brief The Experiment class
 */
class Experiment : public QObject
{
      Q_OBJECT

   public:
      explicit Experiment(int globalIndex, int numTrials,
                          std::weak_ptr<DataReaderWriter> wpDataRW,
                          QObject* parent = nullptr);

      QString getExperimentName() const;
      void setExperimentName(const QString& experimentName);

      QString getPersonID() const;
      void setPersonID(const QString& getPersonID);

      virtual QMap<QString, QVariant> getDataToSave() = 0;
      virtual void setLoadedData(const QMap<QString, QVariant>& data) = 0;

      virtual void start() = 0;
      virtual void pause() = 0;
      virtual void togglePause() = 0;
      virtual void stop() = 0;

      int getGlobalIndex() const;
      int getDataSetCount() const;

      static QString convertColorToString(Qt::GlobalColor color, bool german=false);
      static Qt::GlobalColor convertStringToColor(const QString& colorStr);

      static QString convertColorForStylesheet(Qt::GlobalColor color);

      void setNumTrials(int nNumTrials);

signals:
      void started(int idx);
      void stopped(int idx);

   protected:
      int m_nGlobalIndex;
      int m_nNumTrials;
      int m_nDataSetCount;
      bool m_bStarted;
      bool m_bPaused;
      bool m_bStopped;
      QString m_strExperimentName;
      QString m_strPersonID;
      std::weak_ptr<DataReaderWriter> m_wpDataRW;
};
