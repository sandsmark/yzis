/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <qfile.h>
#include <string>
#include "PhilAsserts.h"
using namespace CppUnit;
using namespace std;

#include "libyzis/line.h"
#include "libyzis/debug.h"
#include "libyzis/buffer.h"
#include "libyzis/buffer.h"

#include "testYZCommands.h"
#include "TYZSession.h"
#include "TYZView.h"


// register the suite so that it is run in the runner
CPPUNIT_TEST_SUITE_REGISTRATION( TestYZCommands );

// some useful macro
#define CHECK_MODE_INSERT( view ) phCheckEquals( view->getCurrentMode(), YZView::YZ_VIEW_MODE_INSERT );
#define CHECK_MODE_COMMAND( view ) phCheckEquals( view->getCurrentMode(), YZView::YZ_VIEW_MODE_COMMAND );
#define CHECK_CURSOR_POS( view, line, col ) { phCheckEquals( view->getCursorLine(), line ); phCheckEquals( view->getCursorCol(), col ); }

/* ========================================================================= */
void TestYZCommands::setUp()
{
    mLines = 5;
    mSession = new TYZSession();
    mBuf = new YZBuffer( mSession );
    mView = new TYZView( mBuf, mSession, mLines );
}

void TestYZCommands::tearDown()
{
    delete mView;
    delete mBuf;
    delete mSession;
}

void TestYZCommands::testCreateSession()
{
    phCheckEquals( mBuf->views().count(), 1 );
    phCheckEquals( mBuf->firstView(), (YZView *) mView );
    phCheckEquals( mBuf->getWholeText(), "" );
    CHECK_MODE_COMMAND( mView );
    CHECK_CURSOR_POS( mView, 0, 0 );
}

void TestYZCommands::testInsertMode()
{
    mView->sendText( "i" );
    phCheckEquals( mBuf->getWholeText(), "" );
    CHECK_MODE_INSERT( mView );
    CHECK_CURSOR_POS( mView, 0, 0 );

    mView->sendText( "i23" );
    phCheckEquals( mBuf->getWholeText(), "i23" );
    CHECK_MODE_INSERT( mView );
    CHECK_CURSOR_POS( mView, 0, 3 );

    mView->sendText( "\n456" );
    phCheckEquals( mBuf->getWholeText(), "i23\n456" );
    CHECK_MODE_INSERT( mView );
    CHECK_CURSOR_POS( mView, 1, 3 );

    mView->sendText( "<Esc>" );
    phCheckEquals( mBuf->getWholeText(), "i23\n456" );
    CHECK_MODE_COMMAND( mView );
    CHECK_CURSOR_POS( mView, 1, 2 );

    mView->sendText( "<Esc>" );
    phCheckEquals( mBuf->getWholeText(), "i23\n456" );
    CHECK_MODE_COMMAND( mView );
    CHECK_CURSOR_POS( mView, 1, 2 );
}

/* ========================================================================= */
