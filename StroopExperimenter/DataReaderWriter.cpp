/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 1 Apr. 2022                                                *
 *****************************************************************************/
 
#include "DataReaderWriter.h"

#include <iostream>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QTextStream>


/**
 * @brief DataReaderWriter::DataReaderWriter
 * @param parent
 */
DataReaderWriter::DataReaderWriter(QObject* parent)
   : QObject(parent)
{

}


/**
 * @brief DataReaderWriter::loadData
 * @param filename
 * @param targetContainer
 * @return
 */
bool DataReaderWriter::loadData(const QString& filePath, QMap<QString, QVariant>& targetContainer)
{
   // Always UTF-8 in Qt 6
   QSettings settings(filePath, QSettings::IniFormat);

   QStringList allKeys = settings.allKeys();

   for (const QString& key : allKeys)
   {
      targetContainer.insert(key, settings.value(key));
   }

   return true;
}


/**
 * @brief DataReaderWriter::saveData
 * @param filename
 * @param sourceContainer
 * @return
 */
bool DataReaderWriter::saveData(const QString& filePath, const QMap<QString, QVariant>& sourceContainer)
{
   // Always UTF-8 in Qt 6
   QSettings settings(filePath, QSettings::IniFormat);

   QList<QString> keys = sourceContainer.keys();

   for (const QString& key : keys)
   {
      settings.setValue(key, sourceContainer.value(key));
   }

   return true;
}


/**
 * @brief DataReaderWriter::writeCSV
 * @param filePath
 * @param list
 * @return
 */
bool DataReaderWriter::writeCSV(const QString& filePath, const QVector<QStringList>& data)
{
   QSaveFile file(filePath);
   if (file.open(QFile::WriteOnly | QFile::Text))
   {
      QTextStream out(&file);
      out.setEncoding(QStringConverter::Utf8);

      for (const QStringList& row : data)
      {
         //for (const QString& col : row)
         //{
         //   out << col << ",";
         //}
         const int nCols = row.length();
         for (int col=0; col<nCols; col++)
         {
            if (col==(nCols-1))
            {
               out << row.at(col);
            }
            else
            {
               out << row.at(col) << ",";
            }
         }

         out << "\n";
      }

      if (!file.commit())
      {
        QString errorMessage = QString("Cannot write file %1:\n%2.")
                       .arg(QDir::toNativeSeparators(filePath), file.errorString());

        std::cout << errorMessage.toStdString() << std::endl;
        return false;
      }
   }
   else
   {
       QString errorMessage = QString("Cannot open file %1 for writing:\n%2.")
                      .arg(QDir::toNativeSeparators(filePath), file.errorString());

       std::cout << errorMessage.toStdString() << std::endl;
       return false;
   }

   return true;
}
