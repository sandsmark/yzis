/*
    Copyright (c) 2003-2005 Mickael Marchand <marchand@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* St */
#ifndef YZIS_WIN32_GCC
#include <libintl.h>
#endif
#include <locale.h>

/* Qt */
#include <QApplication>
#include <QTimer>
#include <QDateTime>

/* Yzis */
#include "libyzis/translator.h"
#include "libyzis/session.h"
#include "libyzis/view.h"
#include "debug.h"
#include "yzis.h"
#include "qyzis.h"
#include "qsession.h"
#include "buffer.h"


int main(int argc, char **argv) {
	QApplication app(argc, argv);

	app.setOrganizationName("Yzis");
	app.setOrganizationDomain("yzis.org");
	app.setApplicationName("QYzis");

	setlocale( LC_ALL, "");

#ifndef YZIS_WIN32_GCC
	bindtextdomain( "yzis", QString("%1%2").arg( PREFIX ).arg("/share/locale").toUtf8().data() );
	bind_textdomain_codeset( "yzis", "UTF-8" );
	textdomain( "yzis" );
#endif

	Qyzis* mw = new Qyzis();
	mw->show();

	QStringList args = app.arguments();

    YZDebugBackend::instance()->parseRcfile( DEBUGRC_FNAME );
    YZDebugBackend::instance()->parseArgv( args );
    yzDebug() << QDateTime::currentDateTime().toString() << endl;


	YZView* first = NULL;
	YZView* v;
	for ( int i = 1; i < args.count(); ++i ) {
		if ( args.at(i)[0] != '-' ) {
			v = QYZisSession::self()->createBufferAndView( args.at(i) );
			if ( !first) 
				first = v;
		}
	}
	if ( !first ) {
		/* no view opened */
		first = QYZisSession::self()->createBufferAndView();
		first->myBuffer()->openNewFile();
		first->displayIntro();
	}

	QYZisSession::self()->setCurrentView( first );

	QTimer::singleShot(0, mw, SLOT( init() ));

	return app.exec();
}

