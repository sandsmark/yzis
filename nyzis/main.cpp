/*
	  Copyright (c) 2003 Yzis Team <yzis-dev@yzis.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/**
 * $Id$
 */

#include "debug.h"

#include "factory.h"
#include <cstdlib>
#include <cstdio>
#include <csignal>


static void catchsigint(int sig);
static void cleaning(void);
void nyz_init_screen(void);

int
main(int argc, char *argv[])
{

	(void) signal(SIGINT, catchsigint);      /* arrange interrupts to terminate */
	atexit(cleaning);

	(new NYZFactory(argc,argv)) -> event_loop();

	yzError() << "Should never reach this point" << endl;
}

static void cleaning(void)
{
	yzDebug() << "end of nyzis, cleaning" << endl;

	/* ncurses stuff */
	endwin();
	/* other */
	printf("\n"); // prevent prompt to be badly placed after nyzis exits
}

static void catchsigint(int /*sig*/)
{
//	yzDebug() << "^C catched" << endl;
	// ^c catched -> sends an escape char.. 
	NYZFactory::currentView->sendKey( Qt::Key_Escape, 0); }

 
