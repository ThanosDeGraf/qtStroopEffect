/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 15 Dec. 2021                                               *
 *****************************************************************************/
 
#include "Experiment.h"
#include "DataReaderWriter.h"
#include <QEvent>


/**
 * @brief Experiment::Experiment
 * @param parent
 */
Experiment::Experiment(int globalIndex, int numTrials,
                       std::weak_ptr<DataReaderWriter> wpDataRW,
                       QObject* parent)
   : QObject(parent)
   , m_nGlobalIndex(globalIndex)
   , m_nNumTrials(numTrials)
   , m_nDataSetCount(0)
   , m_bStarted(false)
   , m_bPaused(false)
   , m_bStopped(true)
   , m_wpDataRW(wpDataRW)
{
}


/**
 * @brief Experiment::getExperimentName
 * @return
 */
QString Experiment::getExperimentName() const
{
   return m_strExperimentName;
}


/**
 * @brief Experiment::setExperimentName
 * @param experimentName
 */
void Experiment::setExperimentName(const QString& experimentName)
{
   m_strExperimentName = experimentName;
}


/**
 * @brief Experiment::getPersonID
 * @return
 */
QString Experiment::getPersonID() const
{
   return m_strPersonID;
}


/**
 * @brief Experiment::setPersonID
 * @param strPersonID
 */
void Experiment::setPersonID(const QString& strPersonID)
{
   m_strPersonID = strPersonID;
}


/**
 * @brief Experiment::getGlobalIndex
 * @return
 */
int Experiment::getGlobalIndex() const
{
   return m_nGlobalIndex;
}


/**
 * @brief Experiment::getDataSetCount
 * @return
 */
int Experiment::getDataSetCount() const
{
   return m_nDataSetCount;
}


/**
 * @brief Experiment::convertColorToString
 * @param color
 * @return QString representation of the chosen color
 */
QString Experiment::convertColorToString(Qt::GlobalColor color, bool german)
{
   QString colorStr("black");

   switch (color)
   {
      case Qt::black:
         if (german) { colorStr = "schwarz"; }
         break;
      case Qt::white:
         colorStr = german ? "weiß" : "white";
         break;
      case Qt::red:
         colorStr = german ? "rot" : "red";
            break;
      case Qt::green:
         colorStr = german ? "grün" : "green";
         break;
      case Qt::blue:
         colorStr = german ? "blau" : "blue";
         break;
      case Qt::darkGray:
         colorStr = "darkgray";
         break;
      case Qt::gray:
         colorStr = "gray";
         break;
      case Qt::lightGray:
         colorStr = "lightgray";
         break;
      case Qt::cyan:
         colorStr = "cyan";
         break;
      case Qt::magenta:
         colorStr = "magenta";
         break;
      case Qt::yellow:
         colorStr = german ? "gelb" : "yellow";
         break;
      case Qt::darkRed:
         colorStr = "darkred";
         break;
      case Qt::darkGreen:
         colorStr = "darkgreen";
         break;
      case Qt::darkBlue:
         colorStr = "darkblue";
         break;
      case Qt::darkCyan:
         colorStr = "darkcyan";
         break;
      case Qt::darkMagenta:
         colorStr = "darkmagenta";
         break;
      case Qt::darkYellow:
         colorStr = "darkyellow";
         break;
      default:
         break;
   }

   return colorStr;
}


/**
 * @brief Experiment::convertColorForStylesheet
 * @param color
 * @return QString representation of the chosen color meant to be used for stylesheets
 *
 * This method returns a QString representation of the chosen color meant to be used
 * for stylesheets. This means requirements
 *
 * @see https://doc.qt.io/qt-5/qcolorconstants.html
 */
QString Experiment::convertColorForStylesheet(Qt::GlobalColor color)
{
   QString colorStr("black");

   switch (color)
   {
      case Qt::black:
         break;
      case Qt::white:
         colorStr = "white";
         break;
      case Qt::red:
         colorStr = "red";
            break;
      case Qt::green:
         colorStr = "green";
         break;
      case Qt::blue:
         colorStr = "blue";
         break;
      case Qt::darkGray:
         colorStr = "darkgray";
         break;
      case Qt::gray:
         colorStr = "gray";
         break;
      case Qt::lightGray:
         colorStr = "lightgray";
         break;
      case Qt::cyan:
         colorStr = "cyan";
         break;
      case Qt::magenta:
         colorStr = "magenta";
         break;
      case Qt::yellow:
         // See "SVG Colors" in https://doc.qt.io/qt-5/qcolorconstants.html
         // "yellow" is	"#ffff00"
         // "gold" is "#ffd700"
         colorStr = "gold";
         break;
      case Qt::darkRed:
         colorStr = "darkred";
         break;
      case Qt::darkGreen:
         colorStr = "darkgreen";
         break;
      case Qt::darkBlue:
         colorStr = "darkblue";
         break;
      case Qt::darkCyan:
         colorStr = "darkcyan";
         break;
      case Qt::darkMagenta:
         colorStr = "darkmagenta";
         break;
      case Qt::darkYellow:
         colorStr = "darkyellow";
         break;
      default:
         break;
   }

   return colorStr;
}

/**
 * @brief Experiment::convertStringToColor
 * @param colorStr
 * @return
 */
Qt::GlobalColor Experiment::convertStringToColor(const QString& colorStr)
{
        if (colorStr == "white")      { return Qt::white;      }
   else if (colorStr == "red")        { return Qt::red;        }
   else if (colorStr == "green")      { return Qt::green;      }
   else if (colorStr == "blue")       { return Qt::blue;       }
   else if (colorStr == "darkgray")   { return Qt::darkGray;   }
   else if (colorStr == "gray")       { return Qt::gray;       }
   else if (colorStr == "lightgray")  { return Qt::lightGray;  }
   else if (colorStr == "cyan")       { return Qt::cyan;       }
   else if (colorStr == "magenta")    { return Qt::magenta;    }
   else if (colorStr == "yellow")     { return Qt::yellow;     }
   else if (colorStr == "darkred")    { return Qt::darkRed;    }
   else if (colorStr == "darkgreen")  { return Qt::darkGreen;  }
   else if (colorStr == "darkblue")   { return Qt::darkBlue;   }
   else if (colorStr == "darkcyan")   { return Qt::darkCyan;   }
   else if (colorStr == "darkmagenta"){ return Qt::darkMagenta;}
   else if (colorStr == "darkyellow") { return Qt::darkYellow; }
   else  /*(colorStr == "black")*/    { return Qt::black;      }

   return Qt::black;
}


/**
 * @brief Experiment::setNumTrials
 * @param nNumTrials
 */
void Experiment::setNumTrials(int nNumTrials)
{
   m_nNumTrials = nNumTrials;
}
