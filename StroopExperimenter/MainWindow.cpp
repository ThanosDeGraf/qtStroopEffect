/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 31 Mar. 2022                                               *
 *****************************************************************************/
 
#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include "Experimenter.h"
#include "StroopExperiment.h"
#include "StroopExperimentDialog.h"

#include <iostream>
#include <QLabel>
#include <QStringList>
#include <QFileDialog>
#include <QDateTime>


/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(std::weak_ptr<Experimenter> experimenter,
                       QWidget* parent)
   : QMainWindow(parent)
   , m_upUI(new Ui::MainWindow)
   , m_wpExperimenter(experimenter)
   , m_pFileStatusLabel(new QLabel(this))
   , m_pExperimentProgressLabel(new QLabel(this))
{
   /* GUI initialization */
   m_upUI->setupUi(this);
   m_upUI->resultsTableWidget->resizeColumnsToContents();
   this->setWindowTitle(QCoreApplication::applicationName());

   //   m_strlColumnTitles << "Text" << "Angezeigte Farbe"
   //                      << "Gewählte Farbe" << "Übereinstimmung"
   //                      << "Reaktionszeit (ms)";

   /* Labels in status bar with stretch and default text */
   m_upUI->statusBar->addPermanentWidget(m_pFileStatusLabel, 1);
   m_pFileStatusLabel->setText("No experiment file loaded.");

   m_pExperimentProgressLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
   m_upUI->statusBar->addPermanentWidget(m_pExperimentProgressLabel, 1);
   m_pExperimentProgressLabel->setText("No experiment currently running.");

   if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
   {
      spExperimenter->setExperimentNamesList(QStringList("stroop"));
   }
   createExperimentDialog();

   /* Connect signals to slots using function pointers */
   // Menu bar
   connect(m_upUI->actionLoad, &QAction::triggered,
           this, &MainWindow::onActionLoad);
   connect(m_upUI->actionSave, &QAction::triggered,
           this, &MainWindow::onActionSave);
   connect(m_upUI->actionExportCSV, &QAction::triggered,
           this, &MainWindow::onActionExportCSV);
   connect(m_upUI->actionExportCSVStats, &QAction::triggered,
           this, &MainWindow::onActionExportCSVStats);
   connect(m_upUI->actionExportAllCSV, &QAction::triggered,
           this, &MainWindow::onActionExportAllCSV);
//   connect(m_upUI->actionBlockOrder, &QAction::triggered,
//           this, &MainWindow::onActionBlockOrder);
//   connect(m_upUI->actionRandomOrder, &QAction::triggered,
//           this, &MainWindow::onActionRandomOrder);
   connect(m_upUI->actionEvalAllTrials, &QAction::toggled,
           this, &MainWindow::onActionEvalAllTrials);
   connect(m_upUI->actionEvalCorrectTrials, &QAction::toggled,
           this, &MainWindow::onActionEvalCorrectTrials);
   connect(m_upUI->numExpRunsSpinBox, SIGNAL(valueChanged(int)),
           this, SLOT(onNumTrialsSpinBoxValueChanged(int)));
   connect(m_upUI->pubuProband, &QPushButton::clicked,
              this, &MainWindow::onProbandSpecified);

   // Experimenter
   if (std::shared_ptr<Experimenter> spExp = m_wpExperimenter.lock())
   {
      // connect(m_upUI->tabWidget, &QTabWidget::currentChanged,
      //         spExp.get(), &Experimenter::onTabChanged);
      connect(spExp.get(), &Experimenter::experimentLoaded,
              this, &MainWindow::onExperimentLoaded);      
   }

   // Start buttons
   connect(m_upUI->stroopStartButton, &QPushButton::pressed,
           this, &MainWindow::startCurrentExperiment);
           
   setTableWidgetVisuals();
}


/**
 * @brief MainWindow::~MainWindow
 */
MainWindow::~MainWindow()
{
}


/**
 * @brief MainWindow::setTableWidgetVisuals
 */
void MainWindow::setTableWidgetVisuals()
{
   m_upUI->resultsTableWidget->setStyleSheet("QTableWidget { border: none; "
                                             "background-color: #FFFFFF; "
                                             "selection-background-color: #3388FF } "
                                             "QTableWidget::item { color: #000000; } "
                                             "QTableWidget::item:selected { color:#000000; }");

   m_upUI->resultsTableWidget->resizeColumnsToContents();
}


/**
 * @brief MainWindow::resetTableWidget
 */
void MainWindow::resetTableWidget()
{
   const int numRows = m_upUI->resultsTableWidget->rowCount();

   for (int i=0; i<numRows; i++)
   {
      m_upUI->resultsTableWidget->removeRow(0);
   }
}


/**
 * @brief MainWindow::onProbandSpecified
 */
void MainWindow::onProbandSpecified()
{
   QString proband = m_upUI->liedProband->text().simplified().replace(" ", "");
   if (proband.isEmpty()) { return; }

   QString fileName = proband + ".stroop";

   QString filePath =
      QFileDialog::getExistingDirectory(this,
                                        "Ordner für Experimentedatei angeben...",
                                        QCoreApplication::applicationDirPath());

   if (!filePath.isEmpty()) { filePath += QDir::separator(); }

   if (std::shared_ptr<Experimenter> spExp = m_wpExperimenter.lock())
   {
      /*bool loaded = */spExp->loadExperiment(fileName);
   }

   resetTableWidget();
}


/**
 * @brief MainWindow::onActionLoad
 */
void MainWindow::onActionLoad()
{
   QString fileName = QFileDialog::getOpenFileName(this, "Load Experiment",
                                                   QCoreApplication::applicationDirPath(),
                                                   "Experiments (*.stroop)");

   if (std::shared_ptr<Experimenter> spExp = m_wpExperimenter.lock())
   {
      /*bool loaded = */spExp->loadExperiment(fileName);
   }

   resetTableWidget();
}


/**
 * @brief MainWindow::onActionSave
 */
void MainWindow::onActionSave()
{
   if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
   {
      QPair<bool, QStringList> info = spExperimenter->getLastLoadedExperimentInfo();

      const QString& personID = info.second.at(0);
      const QString& expType = info.second.at(1);

      QFileInfo fileInfo(info.second.at(2));
      QString filePath = fileInfo.absolutePath()
                       + QDir::separator() + personID + "." + expType;

      QString fileName = QFileDialog::getSaveFileName(this, "Save Experiment",
                                                      QCoreApplication::applicationDirPath(),
                                                      "Experiments (*.stroop)");

      const QString expName("stroop");
      spExperimenter->saveExperiment(fileName,expName);
   }
}


/**
 * @brief MainWindow::exportCSV
 * @param includeStats
 */
void MainWindow::exportCSV(bool includeStats)
{
   // Functionality like this should be placed further down the hierarchy
   if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
   {
      QPair<bool, QStringList> info = spExperimenter->getLastLoadedExperimentInfo();

      const QString& personID = info.second.at(0);

      QFileInfo fileInfo(info.second.at(2));
      QString filePath = fileInfo.absolutePath()
                       + QDir::separator() + personID + ".csv";

      QString fileName = QFileDialog::getSaveFileName(this, "Export CSV", filePath,
                                                      "CSV (*.csv)");

      QStringList headers;
      int numCols = m_upUI->resultsTableWidget->columnCount();
      for(int col=0; col<numCols; col++)
      {
        headers.append(m_upUI->resultsTableWidget->horizontalHeaderItem(col)->text());
      }

      QVector<QStringList> dataToExport;
      if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
      {
         std::shared_ptr<StroopExperiment> spExp =
               std::static_pointer_cast<StroopExperiment>(spExperimenter->getExperiment("stroop"));

         if (spExp) { dataToExport = spExp->exportLastRunToCSV(headers, includeStats); }
      }

      spExperimenter->exportCSV(fileName, dataToExport);
   }
}


/**
 * @brief MainWindow::onActionExportCSV
 */
void MainWindow::onActionExportCSV()
{
   exportCSV(false);
}


/**
 * @brief MainWindow::onActionExportCSVStats
 */
void MainWindow::onActionExportCSVStats()
{
   exportCSV(true);
}


/**
 * @brief MainWindow::onActionExportAllCSV
 */
void MainWindow::onActionExportAllCSV()
{
   // Functionality like this should be placed further down the hierarchy!
   if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
   {
      QPair<bool, QStringList> info = spExperimenter->getLastLoadedExperimentInfo();

      const QString& personID = info.second.at(0);

      QFileInfo fileInfo(info.second.at(2));
      QString filePath = fileInfo.absolutePath()
                       + QDir::separator() + personID + ".csv";

      QString fileName = QFileDialog::getSaveFileName(this, "Export All Experiments to CSV", filePath,
                                                      "CSV (*.csv)");

      QStringList headers;
      int numCols = m_upUI->resultsTableWidget->columnCount();
      for(int col=0; col<numCols; col++)
      {
         headers.append(m_upUI->resultsTableWidget->horizontalHeaderItem(col)->text());
      }

      std::shared_ptr<StroopExperiment> spExp =
            std::static_pointer_cast<StroopExperiment>(spExperimenter->getExperiment("stroop"));

      if (spExp) { spExp->exportAllExperimentsToCSV(fileName, headers); }
   }
}


/**
 * @brief MainWindow::onActionEvalAllTrials
 * @param checked
 */
void MainWindow::onActionEvalAllTrials(bool checked)
{
   if (checked)
   {
      m_upUI->actionEvalCorrectTrials->setChecked(false);

      if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
      {
         std::shared_ptr<StroopExperiment> exp =
                     std::static_pointer_cast<StroopExperiment>(spExperimenter->getExperiment("stroop"));

         exp->activateEvalAllTrialsMode();
      }
   }
}


/**
 * @brief MainWindow::onActionEvalCorrectTrials
 * @param checked
 */
void MainWindow::onActionEvalCorrectTrials(bool checked)
{
   if (checked)
   {
      m_upUI->actionEvalAllTrials->setChecked(false);

      if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
      {
         std::shared_ptr<StroopExperiment> exp =
                     std::static_pointer_cast<StroopExperiment>(spExperimenter->getExperiment("stroop"));

         exp->activateEvalCorrectTrialsOnlyMode();
      }
   }
}


/**
 * @brief MainWindow::onActionEqualDistOrder
 */
//void MainWindow::onActionEqualDistOrder()
//{
//   m_upUI->actionRandomOrder->setChecked(false);

//   if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
//   {
//      std::shared_ptr<StroopExperiment> spExp =
//            std::static_pointer_cast<StroopExperiment>(spExperimenter->getExperiment("stroop"));

//      spExp->setIndexCreationMode(true);
//   }
//}


/**
 * @brief MainWindow::onActionFullyRandomOrder
 */
//void MainWindow::onActionFullyRandomOrder()
//{
//   m_upUI->actionBlockOrder->setChecked(false);

//   if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
//   {
//      std::shared_ptr<StroopExperiment> spExp =
//            std::static_pointer_cast<StroopExperiment>(spExperimenter->getExperiment("stroop"));

//      spExp->setIndexCreationMode(false);
//   }
//}


/**
 * @brief MainWindow::onExperimentLoaded
 * @param id
 */
void MainWindow::onExperimentLoaded()
{
   //   int numTabs = m_upUI->tabWidget->count();
   //   for (int i=0; i<numTabs; i++)
   //   {
   //      m_upUI->tabWidget->setTabEnabled(i,true);
   //   }

   m_upUI->tabWidget->setCurrentIndex(0);

   if (std::shared_ptr<Experimenter> spExp = m_wpExperimenter.lock())
   {
      QPair<bool,QStringList> loaded = spExp->getLastLoadedExperimentInfo();

      QString pathText = loaded.second.last();
      if (pathText.length() > 50) { pathText = QString("...%1").arg(pathText.right(50)); }
      m_pFileStatusLabel->setText(pathText);

      m_upUI->liedProband->setText(spExp->getExperiment("stroop")->getPersonID());

      m_pExperimentProgressLabel->setText(loaded.second.first());
   }
}


/**
 * @brief MainWindow::onNumTrialsSpinBoxValueChanged
 * @param i
 */
void MainWindow::setNumExperimentRuns(int i)
{
   m_upUI->numExpRunsSpinBox->setValue(i);
}


/**
 * @brief MainWindow::onNumTrialsSpinBoxValueChanged
 * @param i
 */
void MainWindow::onNumTrialsSpinBoxValueChanged(int i)
{
   if (std::shared_ptr<Experimenter> spExp = m_wpExperimenter.lock())
   {
      spExp->setNumTrials(i);
   }
}


/**
 * @brief MainWindow::startCurrentExperiment
 */
void MainWindow::startCurrentExperiment()
{
   // Make sure to have the currently specified value set.
   if (std::shared_ptr<Experimenter> spExp = m_wpExperimenter.lock())
   {
      spExp->setNumTrials(m_upUI->numExpRunsSpinBox->value());
   }

   m_spStroopExperimentDialog->showFullScreen();
}


/**
 * @brief MainWindow::onStroopAssessed
 * @param numMatches
 * @param numWrong
 * @param numTotal
 */
void MainWindow::onStroopAssessed(int numMatches, int numWrong, int numTotal, double mean, double stDev)
{
   m_pExperimentProgressLabel->setText(
            QString("Experimente: %1 / Korrekt: %2 / Falsch: %3 / Mittelwert RT: %4s / STD RT: %5s")
               .arg(numTotal).arg(numMatches).arg(numWrong)
               .arg(QString::number(mean/1000.0, 'f', 3),
                    QString::number(stDev/1000.0, 'f', 3)));

   showResultsInTable();

   m_upUI->tabWidget->setCurrentIndex(m_upUI->tabWidget->indexOf(m_upUI->resultsTab));
}


/**
 * @brief MainWindow::showResultsInTable
 */
void MainWindow::showResultsInTable()
{
   resetTableWidget(); // Delete all rows completely, not just the contents

   QVector<QStringList> results;
   bool showCorrectOnly = false; 
   if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
   {
      std::shared_ptr<StroopExperiment> spExp =
            std::static_pointer_cast<StroopExperiment>(spExperimenter->getExperiment("stroop"));

      results = spExp->currentExperimentSetToString(true);
      showCorrectOnly = spExp->getEvalCorrectTrialsOnly();
   }

   int rows = results.count();
   int cols = m_upUI->resultsTableWidget->columnCount();

   int row = 0;
   for (int i=0; i<rows; i++)
   {
      const QStringList& result = results.at(i);

      if (showCorrectOnly && result.at(4) == QString("0")) // "4" is index of serialized "m_bCorrect"
      {
         continue;
      }

      m_upUI->resultsTableWidget->insertRow(row);

      Q_ASSERT(result.count() == cols);

      for (int col=0; col<cols; col++)
      {
         m_upUI->resultsTableWidget->setItem(row, col, // add 1 to skip valid
                                             new QTableWidgetItem(result.at(col)));
      }

      row++;
   }   

   setTableWidgetVisuals(); // Better safe than sorry...
}


/**
 * @brief MainWindow::createExperimentDialog
 */
void MainWindow::createExperimentDialog()
{
   if (std::shared_ptr<Experimenter> spExperimenter = m_wpExperimenter.lock())
   {
      std::shared_ptr<StroopExperiment> spExp =
            std::static_pointer_cast<StroopExperiment>(spExperimenter->getExperiment("stroop"));

      connect(spExp.get(), &StroopExperiment::statsComputed,
              this, &MainWindow::onStroopAssessed);

      m_spStroopExperimentDialog = std::make_shared<StroopExperimentDialog>(spExp);
   }
}

// https://piktochart.com/blog/5-psychology-studies-that-tell-us-how-people-perceive-visual-information/
