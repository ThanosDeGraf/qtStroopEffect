/*****************************************************************************
* Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen   *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 15 Jan. 2022                                               *
 *****************************************************************************/
 
#pragma once

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


// Forward declarations
class QLabel;
class Experimenter;
class ExperimentDialog;


/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
      Q_OBJECT

   public:
      MainWindow(std::weak_ptr<Experimenter> experimenter, QWidget* parent = nullptr);
      ~MainWindow();

   public slots:
      void setNumExperimentRuns(int i);

   private slots:
      void onProbandSpecified();
      void onActionLoad();
      void onActionSave();
      void onActionExportCSV();
      void onActionExportCSVStats();
      void onActionExportAllCSV();
      //void onActionEqualDistOrder();
      //void onActionFullyRandomOrder();
      void onActionEvalAllTrials(bool checked);
      void onActionEvalCorrectTrials(bool checked);
      void onExperimentLoaded();
      void startCurrentExperiment();
      void onStroopAssessed(int numMatches, int numWrong, int numTotal, double mean, double stDev);
      void onNumTrialsSpinBoxValueChanged(int i);

   private:
      // Methods
      void createExperimentDialog();
      void showResultsInTable();
      void setTableWidgetVisuals();
      void resetTableWidget();
      void exportCSV(bool includeStats);

      // References to UI implementation and logic (Experiments meta class)
      std::unique_ptr<Ui::MainWindow> m_upUI;
      std::weak_ptr<Experimenter> m_wpExperimenter;

      // Pointers managed by Qt via parenting
      QLabel* m_pFileStatusLabel;
      QLabel* m_pExperimentProgressLabel;

      // Other variables
      std::shared_ptr<class StroopExperimentDialog> m_spStroopExperimentDialog;
      //      QStringList m_strlColumnTitles;
};
