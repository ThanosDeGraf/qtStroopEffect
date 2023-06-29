/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 29 Nov. 2021                                               *
 *****************************************************************************/
 
#pragma once

#include <QObject>

/**
 * @brief The DataReaderWriter class
 */
class DataReaderWriter : public QObject
{
      Q_OBJECT

   public:
      explicit DataReaderWriter(QObject* parent = nullptr);


   public slots:
      bool loadData(const QString& filePath,
                    QMap<QString, QVariant>& targetContainer);

      bool saveData(const QString& filePath,
                    const QMap<QString, QVariant>& sourceContainer);

      bool writeCSV(const QString& filePath, const QVector<QStringList>& data);

   signals:
      void finishedLoading();
};
