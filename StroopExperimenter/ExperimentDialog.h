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

#include <QDialog>

class Experiment;


/**
 * @brief The ExperimentDialog class
 */
class ExperimentDialog : public QDialog
{
   public:
      explicit ExperimentDialog(QWidget* parent = nullptr);

      virtual int getGlobalExperimentIndex() const; // = 0;
};
