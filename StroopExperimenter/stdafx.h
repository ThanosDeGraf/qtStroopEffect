/*****************************************************************************
 * Copyright (C) 2020-2022 Chair of Media Informatics, University of Siegen  *
 * https://mi.informatik.uni-siegen.de/sites/about.php                       *
 *                                                                           *
 * This file is part of the StroopExperimenter.                              *
 *                                                                           *
 * Author: Thomas Klinkert                                                   *
 * Revision date: 1 Apr. 2022                                                *
 *****************************************************************************/
 
// stdafx.h : Include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.
// Btw.: These are the "Standard Application Framework Extensions"

// GCC complains "warning: #pragma once in main file #pragma once",
// so a classic include guard is used here.
#pragma once
//#ifndef StdAFX_h
//#define StdAFX_h

// C++11 smart pointers
#include <memory>

// Often used Qt headers
#include <QPointer>
#include <QPair>
#include <QString>
#include <QVariant>

#ifdef QT_DEBUG
   #include <QDebug>
#endif

// Definitions
//#define 

//#endif
