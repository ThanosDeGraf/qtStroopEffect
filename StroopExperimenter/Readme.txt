/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 31 Mar. 2022                                               *
 *****************************************************************************/
 
Dieses Projekt benötigt Qt 5 (5.12.x oder neuer) und ist für die Verwendung mit 
dem zu der verwendeten Qt-Version passenden IDE "QtCreator" erstellt worden.

Kommandozeilenparameter:

-f <Name>.jperc      Experimentdatei, die bei Start verwendet werden soll. 
                     <Name> bezeichnet die/den Proband*in. 
                     Z.B. "-f Alex.stroop"
                     Default: "default.stroop"
-o <Ordnerpfad>      Ordner, in dem die Experimentdatei *.stroop gespeichert 
                     werden soll. Z.B. "-o "E:\Data"
                     Default: Projektordner
-n <Anzahl Trials>   Z.B. "-n 100"
