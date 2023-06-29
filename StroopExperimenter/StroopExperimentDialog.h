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

#include "ExperimentDialog.h"


// Forward declarations
class StroopExperiment;
class QKeyEvent;
class QLabel;


/**
 * @brief The StroopExperimentDialog class
 */
class StroopExperimentDialog : public ExperimentDialog
{
      Q_OBJECT

   public:
      StroopExperimentDialog(std::weak_ptr<StroopExperiment> wpExperiment,
                             QWidget* parent = nullptr);

      virtual int getGlobalExperimentIndex() const;

   protected:
      virtual void keyPressEvent(QKeyEvent* evt);
      virtual void showEvent(QShowEvent* evt);
      virtual void closeEvent(QCloseEvent* evt);

   private slots:
      void drawFixationPoint();
      void drawColoredWriting(const QString& text, Qt::GlobalColor color);
      void drawColoredQuad(Qt::GlobalColor color);

   private:
      std::weak_ptr<StroopExperiment> m_wpExperiment;
      QLabel* m_pMainLabel;
};
