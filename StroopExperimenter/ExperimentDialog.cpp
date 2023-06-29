/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 29 Nov. 2021                                               *
 *****************************************************************************/
 
#include "ExperimentDialog.h"
#include "Experiment.h"


/**
 * @brief ExperimentDialog::ExperimentDialog
 * @param parent
 */
ExperimentDialog::ExperimentDialog(QWidget* parent)
   : QDialog(parent)
{
}

int ExperimentDialog::getGlobalExperimentIndex() const
{
   return -1;
}

