/*****************************************************************************
* Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen   *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 1 Apr. 2022                                                *
 *****************************************************************************/
 
#include "StroopExperiment.h"

#include <random>
#include <numeric>
#include <QTimer>
#include <QDateTime>
#include <QObject>


/**
 * @brief StroopTrial::StroopTrial
 */
StroopTrial::StroopTrial()
   : m_bValid(false)
   , m_nMode(StroopTrialModes::ColoredQuads)
   , m_nColor(Qt::black)
   , m_nChosenColor(Qt::black)
   , m_i64DecisionTime(-1LL)
   , m_bCorrect(false)
{
}


/**
 * @brief StroopTrial::StroopTrial
 * @param text
 * @param color
 */
StroopTrial::StroopTrial(StroopTrialModes mode, const QString& text, Qt::GlobalColor color)
   : m_bValid(false)
   , m_nMode(mode)
   , m_strText(text)
   , m_nColor(color)
   , m_nChosenColor(Qt::black)
   , m_i64DecisionTime(-1LL)
   , m_bCorrect(false)
{
}


/**
 * @brief StroopTrial::toString
 * @return
 */
QStringList StroopTrial::toStringList(bool includeValidState, bool german) const
{
   QStringList result;

   if (german)
   {
      if (includeValidState) { result.append(m_bValid ? "valid" : "invalid"); }
   }
   else
   {
      if (includeValidState) { result.append(m_bValid ? "gültig" : "ungültig"); }
   }

   switch(m_nMode)
   {
      case StroopTrialModes::ColoredQuads:
      {
         result.append("Quads");
         break;
      }
      case StroopTrialModes::ColoredTextMatched:
      {
         result.append("TextMatch");
         break;
      }
      case StroopTrialModes::ColorTextConflicted:
      {
         result.append("TextConflict");
         break;
      }
      case StroopTrialModes::ColoredTextUnreferenced:
      {
         result.append("TextUnref");
         break;
      }
      default: { break; }
   }

   result.append(m_strText);
   result.append(Experiment::convertColorToString(m_nColor,german));

   result.append(Experiment::convertColorToString(m_nChosenColor,german));
//   if (german)
//   {
      result.append(m_bCorrect ? "1" : "0");
//   }
//   else
//   {
//      result.append(m_bCorrect ? "match" : "wrong");
//   }
   result.append(QString::number(m_i64DecisionTime/1000.0, 'f', 3));

   return result;
}


/**
 * @brief StroopTrial::toString
 * @param includeValidState
 * @param german
 * @return
 */
QString StroopTrial::toString(bool includeValidState, bool german) const
{
   return toStringList(includeValidState, german).join("&");
}


/**
 * @brief StroopExperiment::StroopExperiment
 */
StroopExperiment::StroopExperiment(int globalIndex, int numTrials,
                                   std::weak_ptr<DataReaderWriter> wpDataRW,
                                   QObject* parent)
   : Experiment(globalIndex, numTrials, wpDataRW, parent)
   , m_nProgress(0)
   , m_bIndexCreationMode(true)
    , m_bEvalCorrectTrialsOnly(false)
    , m_skipped(false)
{
   createStroopTrials();

   timer.setSingleShot(true);
   timer.setInterval(2000);

   timer.start();
   timer.stop();

   connect(&timer, &QTimer::timeout, this, &StroopExperiment::storeTimeAndContinue);
}


/**
 * @brief StroopExperiment::start
 */
void StroopExperiment::start()
{
   // Set states
   if (m_bPaused)
   {
      m_bPaused = false;
   }
   else if (!m_bStarted)
   {
      m_bStarted = true;
      m_bStopped = false;

      // ...and initialize variables specific to each run.
      m_nProgress = 0;

      // Clear temporary result containers
      m_qvecStroopTrialIndices.clear();

      if (m_bIndexCreationMode)
      {
         createEquallyDistributedTrialIndices();
      }
      else
      {
         createFullyRandomTrialIndices();
      }

      m_strLastExpTimeStamp = QDateTime::currentDateTime().toString("yyyy.MM.dd-hh::mm::ss");

      emit started(m_nGlobalIndex);
   }

   startNextTrial();
}


/**
 * @brief StroopExperiment::startNewRun
 * A "new run" consists of displaying the fixation point for 1 second and
 * then showing one experiment screen.
 */
void StroopExperiment::startNextTrial()
{
   if (m_nProgress >= m_nNumTrials)
   {
      stop();
      return; // Yes, break the cycle here.
   }

   emit requestFixationPoint();
   QTimer::singleShot(1000, this, SLOT(issueDisplayRequest()));
}


/**
 * @brief StroopExperiment::issueDisplayRequest
 */
void StroopExperiment::issueDisplayRequest()
{
   if (m_nProgress >= m_nNumTrials)
   {
      stop();
      return;
   }

   StroopTrial curTrial = m_qvecStroopTrials.at(m_qvecStroopTrialIndices.at(m_nProgress));

   if (curTrial.m_nMode == StroopTrialModes::ColoredQuads)
   {
      m_eltiSingleDecisionTime.start(); // also restarts
      emit requestColoredQuad(curTrial.m_nColor);
   }
   else
   {
      m_eltiSingleDecisionTime.start(); // also restarts
      emit requestColoredWriting(curTrial.m_strText, curTrial.m_nColor);
   }
}


/**
 * @brief StroopExperiment::pause
 */
void StroopExperiment::pause()
{
   m_bPaused = true;
}


/**
 * @brief StroopExperiment::togglePause
 */
void StroopExperiment::togglePause()
{
   if(m_bPaused)
   {
      start();
   }
   else
   {
      pause();
   }
}


/**
 * @brief StroopExperiment::stop
 */
void StroopExperiment::stop()
{
   if (!m_bStopped)
   {
      m_bStarted = false;
      m_bPaused  = false;
      m_bStopped = true;

      checkIfAborted();
      evaluateTrials();
      serializeCurrentExperiment();

      emit stopped(m_nGlobalIndex);
   }
}


/**
 * @brief StroopExperiment::storeTimeAndContinue
 */
void StroopExperiment::storeTimeAndContinue()
{
   if(m_skipped) {
      // Save the time needed for the decision
      m_qvecStroopTrials[m_qvecStroopTrialIndices.at(m_nProgress)].m_i64DecisionTime
          = m_eltiSingleDecisionTime.elapsed();

      // Mark the current trial as valid
      m_qvecStroopTrials[m_qvecStroopTrialIndices.at(m_nProgress)].m_bValid = true;

      // Finally, progress to the next trial
      m_nProgress++;

      if (m_bStarted && !m_bPaused)
      {
         startNextTrial();
      }
   }
}


/**
 * @brief StroopExperiment::checkIfAborted
 */
void StroopExperiment::checkIfAborted()
{
   const int maxIdx = m_nNumTrials - 1;

   // "m_nProgress" is current idx, i.e. the index of the aborted trial.
   // Thus, the trial at index "m_nProgress" and all following need to be deleted.
   // Example: progress    5 -> [0,5] -> currentIndex 5
   //          numTrials   8 -> [0,7] -> maxIndex     7
   //  -> delete current invalid one and rest means delete trials at indices 5, 6 and 7.

   // All remaining trials, including the one that was active, are deleted.
   if (m_nProgress < maxIdx)
   {
      for(int i=m_nProgress; i<m_nNumTrials; i++)
      {
         m_qvecStroopTrialIndices.removeLast();
      }

      int numDefinedIndices = m_qvecStroopTrialIndices.count();
      Q_ASSERT(m_nProgress == numDefinedIndices);

      m_nNumTrials = numDefinedIndices;
   }
}


/**
 * @brief StroopExperiment::evaluateTrials
 */
void StroopExperiment::evaluateTrials()
{
   /* Check for each trial if the participant has given the correct response */
   int numCorrect = 0, numWrong = 0;
   qint64 sumDT = 0LL;
   int numDTs = 0;

   std::vector<double> diffMean; // result vector

   int numUsedTrials = 0;
   for (int i=0; i<m_nNumTrials; i++)
   {
      StroopTrial& trial = m_qvecStroopTrials[m_qvecStroopTrialIndices.at(i)];

      if (trial.m_bValid) { numUsedTrials++; }
      else { continue; }

      bool correct = ( trial.m_nColor == trial.m_nChosenColor );
      if (correct) { numCorrect++; } else { numWrong++; }

      trial.m_bCorrect = correct;

      if (!m_bEvalCorrectTrialsOnly || correct)
      {
         sumDT += trial.m_i64DecisionTime;
         numDTs++;

         // Save the individual minuends for later
         diffMean.push_back(static_cast<double>(trial.m_i64DecisionTime));
      }
   }

   Q_ASSERT((numCorrect+numWrong) == numUsedTrials);

   /** Compute mean and standard deviation of the decision time (DT) **/

   /* Mean of decision times */
   double meanDT = static_cast<double>(sumDT) / static_cast<double>(numDTs);

   /* Standard deviation of decision times */

   // Compute differences from the mean for each decision time
   for (int i=0; i<numDTs; i++)
   {
      diffMean[i] = diffMean[i] - meanDT;
   }

   // Sum of squares of differences from mean using inner (dot) product.
   // The differences from the mean are squared by using "diffMean" as argument for both vectors.
   // Last argument is the value to initialize the sum.
   double sqSum = std::inner_product(diffMean.begin(), diffMean.end(),
                                     diffMean.begin(), 0.0);

   // Finally, the standard deviation is computed as:
   double stDevDT = std::sqrt(sqSum / static_cast<double>(numDTs));

   // Save whole assessment as a list of strings
   m_strlLastStats.clear();
   m_strlLastStats = statsToStringList(meanDT, numCorrect, numWrong, stDevDT);

   emit statsComputed(numCorrect, numWrong, m_nNumTrials, meanDT, stDevDT);
}


/**
 * @brief StroopExperiment::assessmentToStringList
 * @param meanRT
 * @param numMatches
 * @param numWrong
 * @param stDevRT
 */
QStringList StroopExperiment::statsToStringList(double meanRT, int numMatches,
                                                int numWrong, double stDevRT, bool german)
{
   QStringList stats;

   if (german)
   {
      stats.append(QString("#Durchgeführte Trials: ") + QString::number(m_nNumTrials));
      stats.append(QString("#Korrekt: ") + QString::number(numMatches));
      stats.append(QString("#Falsch: ") + QString::number(numWrong));
      stats.append(QString("Mittelwert: %1(s)")
                                  .arg(QString::number(meanRT/1000.0, 'f', 3)));
      stats.append(QString("Standardabweichung: %1(s)")
                                  .arg(QString::number(stDevRT/1000.0, 'f', 3)));
   }
   else
   {
      stats.append(QString("#executed trials: ") + QString::number(m_nNumTrials));
      stats.append(QString("#correct: ") + QString::number(numMatches));
      stats.append(QString("#wrong: ") + QString::number(numWrong));
      stats.append(QString("mean: %1(s)")
                                  .arg(QString::number(meanRT/1000.0, 'f', 3)));
      stats.append(QString("standard deviation: %1(s)")
                                  .arg(QString::number(stDevRT/1000.0, 'f', 3)));
   }

   return stats;
}


/**
 * @brief StroopExperiment::getIndexCreationMode
 * @return
 */
bool StroopExperiment::getIndexCreationMode() const
{
   return m_bIndexCreationMode;
}


/**
 * @brief StroopExperiment::setBlockOrderMode
 * @param blockOrderMode
 */
void StroopExperiment::setIndexCreationMode(bool blockOrderMode)
{
   if (m_bStopped)
   {
      m_bIndexCreationMode = blockOrderMode;
   }
}


/**
 * @brief StroopExperiment::activateEvalCorrectTrialsOnlyMode
 */
void StroopExperiment::activateEvalCorrectTrialsOnlyMode()
{
   m_bEvalCorrectTrialsOnly = true;
}


/**
 * @brief JpegPerceptionExperiment::activateEvalAllTrialsMode
 */
void StroopExperiment::activateEvalAllTrialsMode()
{
   m_bEvalCorrectTrialsOnly = false;
}


/**
 * @brief StroopExperiment::getLastAssessmentStringList
 * @return
 */
QStringList StroopExperiment::getLastStatsStringList() const
{
   return m_strlLastStats;
}


/**
 * @brief StroopExperiment::onRedChosen
 */
void StroopExperiment::onRedChosen()
{
   m_qvecStroopTrials[m_qvecStroopTrialIndices.at(m_nProgress)].m_nChosenColor = Qt::red;

   storeTimeAndContinue();
}


/**
 * @brief StroopExperiment::onGreenChosen
 */
void StroopExperiment::onGreenChosen()
{
   m_qvecStroopTrials[m_qvecStroopTrialIndices.at(m_nProgress)].m_nChosenColor = Qt::green;

   storeTimeAndContinue();
}


/**
 * @brief StroopExperiment::onBlueChosen
 */
void StroopExperiment::onBlueChosen()
{
   m_qvecStroopTrials[m_qvecStroopTrialIndices.at(m_nProgress)].m_nChosenColor = Qt::blue;

   storeTimeAndContinue();
}


/**
 * @brief StroopExperiment::onYellowChosen
 */
void StroopExperiment::onYellowChosen()
{
   m_qvecStroopTrials[m_qvecStroopTrialIndices.at(m_nProgress)].m_nChosenColor = Qt::yellow;

   storeTimeAndContinue();
}


/**
 * @brief StroopExperiment::createStroopTrials
 * @todo Check which order is intended / best
 */
void StroopExperiment::createStroopTrials()
{
   // Quads [0-3]
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredQuads, "Rot",  Qt::red));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredQuads, "Grün", Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredQuads, "Blau", Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredQuads, "Gelb", Qt::yellow));

   // Text matches the color [4-7]
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextMatched, "Rot",  Qt::red));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextMatched, "Grün", Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextMatched, "Blau", Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextMatched, "Gelb", Qt::yellow));

   // Text and color are conflicted [8-19]
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Grün", Qt::red));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Blau", Qt::red));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Gelb", Qt::red));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Rot",  Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Blau", Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Gelb", Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Rot",  Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Grün", Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Gelb", Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Rot",  Qt::yellow));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Grün", Qt::yellow));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColorTextConflicted, "Blau", Qt::yellow));

   /* Other words [20-39] */
   // https://www.n-joy.de/leben/11-deutsche-Woerter-die-ihr-garantiert-nicht-kennt,witzigewoerter100.html
   // "Durcheinander, Gerümpel, wertloses Zeug":
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Schurrmurr", Qt::red));   // 20
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Schurrmurr", Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Schurrmurr", Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Schurrmurr", Qt::yellow));

   // https://www.n-joy.de/leben/11-deutsche-Woerter-die-ihr-garantiert-nicht-kennt,witzigewoerter100.html
   // "...beschreibt laut Duden ein "dünnes, gehaltloses, fades Getränk"":
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Plempe", Qt::red));   // 24
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Plempe", Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Plempe", Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Plempe", Qt::yellow));

   // https://sternenvogelreisen.de/selten-schoene-woerter-der-deutschen-sprache/
   // "klarer Sternenhimmel":
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Glanzgefunkel", Qt::red));   // 28
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Glanzgefunkel", Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Glanzgefunkel", Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "Glanzgefunkel", Qt::yellow));

   // https://sternenvogelreisen.de/selten-schoene-woerter-der-deutschen-sprache/
   // "wundersam, erstaunlich"
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "putzwunderlich", Qt::red));   // 32
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "putzwunderlich", Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "putzwunderlich", Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "putzwunderlich", Qt::yellow));

   // https://sternenvogelreisen.de/selten-schoene-woerter-der-deutschen-sprache/
   // "seliger als selig, überaus beglückt:"
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "überselig", Qt::red));   // 36
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "überselig", Qt::green));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "überselig", Qt::blue));
   m_qvecStroopTrials.append(StroopTrial(StroopTrialModes::ColoredTextUnreferenced, "überselig", Qt::yellow));
}


/**
 * @brief StroopExperiment::createRandomStroopTrialIndices
 * @see https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
 * This method produces m_nNumTrials indices in the [0, numStroopTrials-1] range
 * using an uniform integer distribution. Basically, it's like throwing a dice
 * with numStroopTrials sides m_nNumTrials times.
 */
void StroopExperiment::createFullyRandomTrialIndices()
{
   const int numStroopTrials = m_qvecStroopTrials.count();
   const int highestIndex = numStroopTrials-1;

   // Provides the seed for the pseudo random number generator (PRNG)
   // (1) https://en.cppreference.com/w/cpp/numeric/random/random_device
   std::random_device randDevice;

   // "The performance of many implementations of random_device degrades sharply
   // once the entropy pool is exhausted. For practical use random_device is
   // generally only used to seed a PRNG such as mt19937." (1)

   // Standard "Mersenne Twister" PRNG engine seeded by randDevice whose
   // "operator() [...] advances the engine's state and returns the generated value". (1)
   std::mt19937 genEngine(randDevice());

   // (2) https://de.wikipedia.org/wiki/Mersenne-Twister
   // The Mersenne Twister PRNG features an extremely low correlation between
   // successive series of values in the output sequence. (2)
   // It is also very fast and often used in non-security related applications.

   // "Produces random integer values i, uniformly distributed on the closed interval [a,b]..."
   // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
   std::uniform_int_distribution<> dist(0, numStroopTrials-1);

   // Delete old indices
   m_qvecStroopTrialIndices.clear();

   int lastIndex = 0;
   for (int i=0; i<m_nNumTrials; ++i)
   {
      // Use the distribution object "dist" to transform the random unsigned int
      // generated by "genEngine" into an int in [0, numStroopTrials-1] and
      // add it to the indices vector.
      int index = dist(genEngine);

      if (index == lastIndex)
      {
         if (index < highestIndex)
         {
            index++;
         }
         else
         {
            index = 0;
         }
      }

      m_qvecStroopTrialIndices.append(index);
   }

   // Shuffle the vector of integers randomly
   //std::random_shuffle(m_qvecStroopTrialIndices.begin(), m_qvecStroopTrialIndices.end());

   //qDebug() << __FUNCTION__ << m_qvecStroopTrialIndices;
}


/**
 * @brief StroopExperiment::createBlockedStroopTrialIndices
 *
 * Creates blocks of random indices for each type of trial.
 * The three types of trials or conditions are:
 * case 1: neutral stimulus: colored quad
 * case 2: no conflict, e.g. "green" written in green
 * case 3: conflict, e.g. "green" written in red
 *
 * @see https://en.cppreference.com/w/cpp/numeric/random
 */
void StroopExperiment::createEquallyDistributedTrialIndices()
{
   // Delete old indices
   m_qvecStroopTrialIndices.clear();

   /* Make four blocks for the three conditions (types of trials) */
   const int numConditions = 4;
   const int runsPerBlock = m_nNumTrials / numConditions;
   const int newTotalNumTrials = runsPerBlock * numConditions;
   const int diffNumTrials = m_nNumTrials - newTotalNumTrials;

   // Increase number of "more fun" experiments if not an equal amount of all three
   // can be instanced to achieve the desired total number of trials (experiment runs).
   int numTrialsCondition1 = runsPerBlock;
   int numTrialsCondition2 = runsPerBlock;
   int numTrialsCondition3 = runsPerBlock;
   int numTrialsCondition4 = runsPerBlock;

   switch (diffNumTrials)
   {
      case 1: { numTrialsCondition4++; break; }
      case 2: { numTrialsCondition4++; numTrialsCondition3++; break; }
      case 3: { numTrialsCondition4++; numTrialsCondition3++; numTrialsCondition2++; break; }
      default: break;
   }

   // Beware! Knowledge about m_qvecStroopTrials and how it set up in
   // createStroopTrials() is used in the following.
   // The blockwise generation is hard-coded using the following index ranges:
   // [0-3]-> quads, [4-7]-> no conflicts, [8-19]-> conflicts, [20-39] other words
   {
      // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
      std::random_device randDevice;
      std::mt19937 genEngine(randDevice());
      std::uniform_int_distribution<> dist(0, 3); // [0-3]-> quads

      int lastIndex = 0;
      for (int i=0; i<numTrialsCondition1; ++i)
      {
         int index = dist(genEngine);

         // Avoid doubling of consecutive indices.
         if (index == lastIndex)
         {
            if (index < 3) { index++; }
            else { index = 0; }
         }

         m_qvecStroopTrialIndices.append(index);
         lastIndex = index;
      }
   }
   {
      std::random_device randDevice;
      std::mt19937 genEngine(randDevice());
      std::uniform_int_distribution<> dist(4, 7); // [4-7]-> no conflicts

      int lastIndex = 4;
      for (int i=0; i<numTrialsCondition2; ++i)
      {
         int index = dist(genEngine);

         // Avoid doubling of consecutive indices.
         if (index == lastIndex)
         {
            if (index < 7) { index++; }
            else { index = 4; }
         }

         m_qvecStroopTrialIndices.append(index);
         lastIndex = index;
      }
   }
   {
      std::random_device randDevice;
      std::mt19937 genEngine(randDevice());
      std::uniform_int_distribution<> dist(8, 19); // [8-19]-> conflicts

      int lastIndex = 8;
      for (int i=0; i<numTrialsCondition3; ++i)
      {
         int index = dist(genEngine);

         // Avoid doubling of consecutive indices.
         if (index == lastIndex)
         {
            if (index < 19) { index++; }
            else { index = 8; }
         }

         m_qvecStroopTrialIndices.append(index);
         lastIndex = index;
      }
   }
   {
      std::random_device randDevice;
      std::mt19937 genEngine(randDevice());
      std::uniform_int_distribution<> dist(20, 39); // [20-39]-> other words

      int lastIndex = 20;
      for (int i=0; i<numTrialsCondition4; ++i)
      {
         int index = dist(genEngine);

         // Avoid doubling of consecutive indices.
         if (index == lastIndex)
         {
            if (index < 39) { index++; }
            else { index = 20; }
         }

         m_qvecStroopTrialIndices.append(index);
         lastIndex = index;
      }
   }

   // Shuffle the vector of integers randomly
   std::mt19937 rng(std::time(nullptr));
   std::shuffle(m_qvecStroopTrialIndices.begin(), m_qvecStroopTrialIndices.end(), rng);
}


/**
 * @brief StroopExperiment::getDataToSave
 * @return
 * @todo Proper handling of required conditions using exeptions.
 */
QMap<QString, QVariant> StroopExperiment::getDataToSave()
{
   return m_mapSerializedResults;
}


/**
 * @brief StroopExperiment::serializeCurrentExperiment
 */
void StroopExperiment::serializeCurrentExperiment()
{
   const bool german = true;
   const int numUsedTrials  = m_qvecStroopTrialIndices.count();

   if (numUsedTrials > 0)
   {
      Q_ASSERT(m_nNumTrials == numUsedTrials);

      // The result map will get one more entry:
      // the list of serialized results of all experiment screens of the last run.
      QStringList allExpData;

      // Add experiment date
      allExpData.append(m_strLastExpTimeStamp);

      for (int i=0; i<m_nNumTrials; i++)
      {
         const StroopTrial& trial = m_qvecStroopTrials.at(m_qvecStroopTrialIndices.at(i));

         if (!trial.m_bValid) { continue; }

         allExpData.append(trial.toString(false, german));
      }

      // Store serialized results
      m_nDataSetCount++;
      QVariant allExpDataVar(allExpData);
      m_mapSerializedResults.insert(QString("StroopResults_%1").arg(m_nDataSetCount), allExpDataVar);
   }
}


/**
 * @brief StroopExperiment::getEvalCorrectTrialsOnly
 * @return
 */
bool StroopExperiment::getEvalCorrectTrialsOnly() const
{
   return m_bEvalCorrectTrialsOnly;
}


/**
 * @brief StroopExperiment::exportLastRunToCSV
 */
QVector<QStringList> StroopExperiment::exportLastRunToCSV(const QStringList& headers,
                                                          bool includeStats=false) const
{
   const bool german = true;
   QVector<QStringList> dataToExport;

   int numPlannedTrials = m_qvecStroopTrialIndices.count();

   if (includeStats)
   {
      QStringList expNameTime(QString("Stroop Experiment:"));
      expNameTime.append(m_strLastExpTimeStamp);


      QStringList stats;
      if (german) { stats.append(QString("#Gültige Trials: ") + QString::number(numPlannedTrials)); }
      else        { stats.append(QString("#Valid trials: ") + QString::number(numPlannedTrials)); }
      stats.append(m_strlLastStats);

      dataToExport.append(expNameTime);
      dataToExport.append(stats);
      dataToExport.append(QStringList("")); // new line
   }


   QVector<QStringList> allExpData;
   int numUsedTrials = 0;
   if (numPlannedTrials > 0)
   {
      Q_ASSERT(m_nNumTrials == numPlannedTrials);

      for (int i=0; i<m_nNumTrials; i++)
      {
         const StroopTrial& trial = m_qvecStroopTrials.at(m_qvecStroopTrialIndices.at(i));

         if (trial.m_bValid) { numUsedTrials++; }
         else { continue; }

         allExpData.append(trial.toStringList(false, german));
      }
   }

   dataToExport.append(headers);
   dataToExport.append(allExpData);

   return dataToExport;
}


/**
 * @brief StroopExperiment::exportAllExperimentsToCSV
 * @param filename
 * @param headers
 * @return
 */
bool StroopExperiment::exportAllExperimentsToCSV(const QString& filename,
                                                 QStringList headers)
{
   // Each QStringList is a row
   QVector<QStringList> dataToExport;

   // Define column headers: add new column headers using
   headers.prepend("Zeitstempel");    // headers.prepend("Time Stamp");
   headers.prepend("Versuchsperson"); // headers.prepend("Participant");
   dataToExport.append(headers);

   // Serialize all stored experiments including the current one
   for (int i=1; i<=m_nDataSetCount; i++)
   {
      const QStringList& allExpData = m_mapSerializedResults.value(QString("StroopResults_%1").arg(i)).toStringList();

      QString dateTime;
      int nextIdx = 1;
      if (allExpData.at(0).contains(":")) // identify date-time string
      {
         dateTime = allExpData.at(0);
      }
      else // if it's not a date-time string, an old file has been loaded
      {
         nextIdx = 0;
      }

      int allExpDataCount = allExpData.count();

      // here are the individual trials
      for (int idx=nextIdx; idx<allExpDataCount; idx++)
      {
         QStringList trialValues;

         trialValues.append(m_strPersonID);
         trialValues.append(dateTime);
         trialValues.append(allExpData.at(idx).split("&"));

         dataToExport.append(trialValues);
      }
   }

   if (std::shared_ptr<DataReaderWriter> spDataRW = m_wpDataRW.lock())
   {
      return spDataRW->writeCSV(filename, dataToExport);
   }
   else
   {
      return false;
   }
}


/**
 * @brief StroopExperiment::currentExperimentSetToString
 * @param german
 * @return
 */
QVector<QStringList> StroopExperiment::currentExperimentSetToString(bool german) const
{
   int numUsedTrials  = m_qvecStroopTrialIndices.count();

   QVector<QStringList> allExpData;
   if (numUsedTrials > 0)
   {
      Q_ASSERT(m_nNumTrials == numUsedTrials);

      for (int i=0; i<m_nNumTrials; i++)
      {
         const StroopTrial& trial = m_qvecStroopTrials.at(m_qvecStroopTrialIndices.at(i));

         if (!trial.m_bValid) { continue; }

         allExpData.append(trial.toStringList(false, german));
      }
   }

   return allExpData;
}


/**
 * @brief StroopExperiment::setLoadedData
 * @param data
 */
void StroopExperiment::setLoadedData(const QMap<QString, QVariant>& data)
{
   m_mapSerializedResults = data;
   m_nDataSetCount = data.count();

   // QStringList allExpData = data.value("StroopResults").toStringList();
   // QStringList newestExpData = data.last().toStringList();

   //for (const QString& expStr : allExpData)
   //{
   //    QStringList expData = expStr.split("-");

   //    Q_ASSERT(expData.count() == 4);

   //    StroopTrial cfg(expData.at(0), static_cast<Qt::GlobalColor>(expData.at(1).toInt()));
   //    m_qvecStroopTrials.append(cfg);

   //    m_qvecChosenColors.append(Experiment::convertStringToColor(expData.at(2)));

   //    m_qvecNeededTimes.append(expData.at(3).toInt());
   //}
}
