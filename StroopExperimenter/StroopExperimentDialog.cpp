/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 20 Dec. 2021                                               *
 *****************************************************************************/
 
#include "StroopExperimentDialog.h"
#include "StroopExperiment.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QKeyEvent>
#include <QColor>
#include <QPainter>


/**
 * @brief StroopExperimentDialog::StroopExperimentDialog
 * @param parent
 */
StroopExperimentDialog::StroopExperimentDialog(
      std::weak_ptr<StroopExperiment> wpExperiment, QWidget* parent)
   : ExperimentDialog(parent)
   , m_wpExperiment(wpExperiment)
   , m_pMainLabel(new QLabel)
{
   // Set up the dialog
   this->setStyleSheet("QDialog { background-color : white; }");

   QHBoxLayout* layoutH = new QHBoxLayout; // Gets a parent later...
   layoutH->addStretch(1);
   layoutH->addWidget(m_pMainLabel);
   layoutH->addStretch(1);

   this->setLayout(layoutH); // Sets "this" as parent of the layout

   // Draw initial state
   drawFixationPoint();

   // Signal/slot connections to the experiment logic
   if (std::shared_ptr<StroopExperiment> spExp = m_wpExperiment.lock())
   {
      connect(spExp.get(), &StroopExperiment::requestFixationPoint,
              this, &StroopExperimentDialog::drawFixationPoint);
      connect(spExp.get(), &StroopExperiment::requestColoredWriting,
              this, &StroopExperimentDialog::drawColoredWriting);
      connect(spExp.get(), &StroopExperiment::requestColoredQuad,
              this, &StroopExperimentDialog::drawColoredQuad);
      connect(spExp.get(), &StroopExperiment::stopped,
              this, &StroopExperimentDialog::close);
   }

}


/**
 * @brief StroopExperimentDialog::showEvent
 * @param evt
 */
void StroopExperimentDialog::showEvent(QShowEvent* evt)
{
   if (std::shared_ptr<StroopExperiment> spExp = m_wpExperiment.lock())
   {
      spExp->start();
   }

   ExperimentDialog::showEvent(evt);
}


/**
 * @brief StroopExperimentDialog::closeEvent
 * @param evt
 */
void StroopExperimentDialog::closeEvent(QCloseEvent* evt)
{
   if (std::shared_ptr<StroopExperiment> spExp = m_wpExperiment.lock())
   {
      spExp->stop(); // Important if closed before the experiment has finished.
   }

   ExperimentDialog::closeEvent(evt);
}


/**
 * @brief StroopExperimentDialog::getGlobalExperimentIndex
 * @return
 */
int StroopExperimentDialog::getGlobalExperimentIndex() const
{
   if (std::shared_ptr<StroopExperiment> spExp = m_wpExperiment.lock())
   {
      return spExp->getGlobalIndex();
   }
   else
   {
      return -1;
   }
}


/**
 * @brief StroopExperimentDialog::keyPressEvent
 * @param evt
 */
void StroopExperimentDialog::keyPressEvent(QKeyEvent* evt)
{
   if (std::shared_ptr<StroopExperiment> spExp = m_wpExperiment.lock())
   {
      // Red
      if(evt->key() == Qt::Key_R || evt->key() == Qt::Key_A || evt->key() == Qt::Key_7)
      {
         spExp->onRedChosen();
         return;
      }
      // Green
      if(evt->key() == Qt::Key_G || evt->key() == Qt::Key_S || evt->key() == Qt::Key_4)
      {
         spExp->onGreenChosen();
         return;
      }
      // Blue
      if(evt->key() == Qt::Key_B || evt->key() == Qt::Key_D || evt->key() == Qt::Key_1)
      {
         spExp->onBlueChosen();
         return;
      }
      // Yellow
      if(evt->key() == Qt::Key_Y || evt->key() == Qt::Key_F || evt->key() == Qt::Key_0)
      {
         spExp->onYellowChosen();
         return;
      }
      // Pause and continue
      if(evt->key() == Qt::Key_Space || evt->key() == Qt::Key_Pause)
      {
         spExp->togglePause();
         return;
      }
   }
   // Stop
   if(evt->key() == Qt::Key_Escape)
   {
      close();
   }

   // If none of the cases are true, let the parent class handle the event.
   QDialog::keyPressEvent(evt);
}


/**
 * @brief StroopExperimentDialog::drawFixationPoint
 */
void StroopExperimentDialog::drawFixationPoint()
{
   m_pMainLabel->setText("+");
   QFont font = m_pMainLabel->font();
   font.setPointSize(128);
   font.setBold(true);
   m_pMainLabel->setFont(font);
   m_pMainLabel->setStyleSheet("QLabel { background-color : white; color : black; }");
}


/**
 * @brief StroopExperimentDialog::drawColoredWriting
 * @param color
 */
void StroopExperimentDialog::drawColoredWriting(const QString& text, Qt::GlobalColor color)
{
   QString colorStr = StroopExperiment::convertColorForStylesheet(color);

   QFont font = m_pMainLabel->font();
   font.setPointSize(96);
   font.setBold(true);
   m_pMainLabel->setFont(font);

   QString styleSheet =
         QString("QLabel { background-color : white; color : %1; }").arg(colorStr);
   m_pMainLabel->setStyleSheet(styleSheet);

   m_pMainLabel->setText(text);
}


/**
 * @brief StroopExperimentDialog::drawColoredRectangle
 * @param color
 */
void StroopExperimentDialog::drawColoredQuad(Qt::GlobalColor color)
{
   /* In order to have the same colors as for the text, the stylesheet of the label  */
   /* that will contain the the pixmap of the quad is set first to retrieve the      */
   /* color to draw the quad. The idea is to have QLabel to interpret the stylesheet */
   /* and then get the color from the m_pMainLabel and use it to draw the QPixmap    */
   /* which contains the quad.                                                       */

   // Define the color of the QLabel "m_pMainLabel" using a stylesheet
   QString colorStr = StroopExperiment::convertColorForStylesheet(color);
   QString styleSheet =
         QString("QLabel { background-color : white; color : %1; }").arg(colorStr);
   m_pMainLabel->setStyleSheet(styleSheet);

   // Get the color the label would use for the text
   QColor labelTextColor = m_pMainLabel->palette().color(QPalette::Text);

   // Create the quad to draw by defining a square QPixmap and set its color
   // to the labelTextColor.
   int quadHeight = this->height() / 4;
   QPixmap quad(quadHeight,quadHeight);
   quad.fill(labelTextColor);

   // Finally draw the quad by setting the pixmap of the m_pMainLabel
   m_pMainLabel->setPixmap(quad);
}
