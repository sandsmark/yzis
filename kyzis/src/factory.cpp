/*  This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
 *  Copyright (C) 2003-2004 Lucijan Busch <lucijan@kde.org>
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

/**
 * $Id$
 */

#include <klocale.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <assert.h>
#include <unistd.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <qstring.h>
#include <ktexteditor/document.h>
#include <kmessagebox.h>
#include <kstaticdeleter.h>
#include <qtimer.h>
#include <qtextcodec.h>

#include "settings.h"
#include "factory.h"
#include "ktexteditoriface.h"
#include "viewwidget.h"
#include "debug.h"
#include "yzis.h"
#include "kyzis.h"
#include "kyzisbuffer.h"

KYZTextEditorIface *KYZisFactory::currentDoc=0;

class KYZisPublicFactory : public KParts::Factory {
	public :
		KParts::Part *createPartObject( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args ) {
			return KYZisFactory::self()->createPartObject( parentWidget, widgetName, parent, name, classname, args );
		}
};

K_EXPORT_COMPONENT_FACTORY( libkyzispart, KYZisPublicFactory)

KYZisFactory::KYZisFactory() :
		KParts::Factory(Kyzis::me),
		YZSession("kyzis"),
		m_aboutData("kyzispart", I18N_NOOP("Kyzis"), VERSION_CHAR, I18N_NOOP("Embeddable vi-like editor component"),
		KAboutData::License_LGPL_V2, I18N_NOOP("(c)2002-2005 The Kyzis Authors"),0,"http://www.yzis.org"),
		m_instance( &m_aboutData ),
		lastMdi(0)
{
	m_aboutData.addAuthor ("Mickael Marchand", I18N_NOOP("Initial Author"), "marchand@kde.org");
	m_aboutData.addAuthor ("Thomas Capricelli", I18N_NOOP("Initial Author"), "orzel@freehackers.org");
	m_aboutData.addAuthor ("Philippe Fremy", I18N_NOOP("Initial Author"), "phil@freehackers.org");
	m_aboutData.addAuthor ("Loic Pauleve", I18N_NOOP("Initial Author"), "panard@inzenet.org");
	m_aboutData.addCredit( "Thomas Nagy", I18N_NOOP( "Icons, KSettings" ), "tnagy@eleve.emn.fr" );
	m_aboutData.addCredit( "Lucijan Bush", I18N_NOOP( "EX mode history" ), "lucijan@kde.org" );
	m_aboutData.addCredit( "Mathieu Kooiman", I18N_NOOP( "KTextEditor interfaces / Dutch translation" ), "M.Kooiman@map-is.nl" );
	m_aboutData.addCredit( "Pascal Maillard", I18N_NOOP( "Commands and motion engine rewrite and cleanup" ), "pascalmaillard@web.de" );
	m_aboutData.addCredit( "Alexander Dymo", I18N_NOOP( "KTextEditor ConfigInterface" ), "adymo@mksat.net" );
	m_aboutData.addCredit( "Erlend Hamberg", I18N_NOOP( "View scrolling fixes" ), "ehamberg@online.no" );
	m_aboutData.addCredit( "Per Johansson", I18N_NOOP( "Kyzis Tab caption improvement" ), "per.j@hjolug.org" );
	m_aboutData.addCredit( "Helder Correia", I18N_NOOP( "Commands and bug fixes" ), "helder.correia@netcabo.pt" );
	m_aboutData.addCredit( "Scott Newton", I18N_NOOP( "Yzisinfo file and tag commands" ), "scottn@ihug.co.nz" );
	m_aboutData.addCredit( "Dmitry Suzdalev", I18N_NOOP( "Misc fixes and improvements" ), "dimsuz@mail.ru" );
	m_aboutData.addCredit( "Dawid Ciezarkiewicz",I18N_NOOP( "Fixed 'xp' command" ), "araelx@gmail.com" );
	m_aboutData.addCredit( "Craig Howard",I18N_NOOP( ":cd and :pwd commands" ), "craig@choward.ca" );
	m_aboutData.addCredit( "Lars Ivar Igesund",I18N_NOOP( "utf8 fixes for Lua strings" ), "larsivar@igesund.net" );
	m_aboutData.addCredit( "Christoph Cullmann",I18N_NOOP( "help to port to KDE4" ), "cullmann@kde.org" );
	m_aboutData.addCredit( "Dawid Ciezarkiewicz",I18N_NOOP( "D language syntax highlight fixes" ), "araelx@gmail.com" );
	m_aboutData.setTranslator(I18N_NOOP("_: NAME OF TRANSLATORS\nYour names"), I18N_NOOP("_: EMAIL OF TRANSLATORS\nYour emails"));

	Settings::self()->readConfig();
	guiStarted();
}

KYZisFactory::~KYZisFactory() {
	kdDebug() << "Factory gets destroyed !" << endl;
}

static KStaticDeleter<KYZisFactory> sdFactory;

KYZisFactory *KYZisFactory::self() {
	static KYZisFactory *self = 0;

	if ( !self ) {
		self = new KYZisFactory;
	}
		
	return self;
}

KParts::Part *KYZisFactory::createPartObject( QWidget *parentWidget, const char *widgetname, QObject *parent, const char *name, const char *_classname, const QStringList &/*args*/) {
	yzDebug() << "Factory::createPartObject" << endl;
	QCString classname (_classname);
	bool bSingleView = (classname!="KTextEditor::Document");
	yzDebug() << "Factory singleView  " << bSingleView << endl;

	if ( parentWidget && parentWidget->inherits( "Kyzis" ) )
		Kyzis::me = static_cast<Kyzis*>( parentWidget );
	else {
		Kyzis::me = 0;
	}

	KYZTextEditorIface *doc = new KYZTextEditorIface( NULL, parentWidget, widgetname, parent, name );
	doc->setReadWrite( true );
	
	return doc;
}

const KAboutData *KYZisFactory::aboutData() {
	return &m_aboutData;
}

bool KYZisFactory::quit( int /*errorCode*/ ) {
	//a kpart CAN NOT exit the main app ;)
	if (Kyzis::me) {
		kapp->quit();
	} else if (kapp->name() == QString::fromLatin1("kdevelop") ) {
		/*
		for (int i = 0; i < YZSession::mNbViews; i++)
			deleteView(i);
		*/
	} else if ( currentView() && currentView()->modePool()->currentType() == YZMode::MODE_EX 
				&& !currentView()->getCommandLineText().isEmpty() ) {
		return false;
	}		


	return true;
}

void KYZisFactory::writeConfig() {
	Settings::self()->writeConfig();
	applyConfig();
}

void KYZisFactory::applyConfig() {
	// apply new configuration to all views
	YZBufferList::ConstIterator it = buffers().begin();
	YZBufferList::ConstIterator end = buffers().end();
	for ( ; it != end; ++it ) {
		YZBuffer *b = *it;
		YZList<YZView*> l = b->views();
		for ( YZList<YZView*>::Iterator itr = l.begin(); itr != l.end(); ++itr ) {
			KYZisView* yv = static_cast<KYZisView*>( *itr );
			yv->applyConfig();
		}
	}
}

void KYZisFactory::readConfig( ) {
	Settings::self()->readConfig();
	applyConfig();
}

void KYZisFactory::changeCurrentView( YZView* view ) {
	yzDebug() << "Kyzis : setCurrentView " << view->getId() << endl;
	KYZisView *v = static_cast<KYZisView*>(view);
	v->setActiveWindow();
	v->setFocus();
}

YZView* KYZisFactory::doCreateView( YZBuffer *buffer ) {
	KYZisBuffer *kbuffer = dynamic_cast<KYZisBuffer*>(buffer);
	KYZTextEditorIface *doc = kbuffer->getTextEditorIface();
	KYZisView *view = 0;
	
	if ( doc ) {
		view = new KYZisView( doc, Kyzis::me, buffer->fileName() + "-view" );
		buffer->addView( view );
		
		if ( Kyzis::me ) {
			KMdiChildView *mdi = Kyzis::me->createWrapper( view, buffer->fileName(), buffer->fileName() );
			view->setMdiChildView( mdi );
			
			Kyzis::me->addWindow( mdi );
			Kyzis::me->createKPartGUI( doc );
		} else {
			view->setMdiChildView( 0 );
		}

		view->setFocus();
		view->setKPart( doc );
		view->show();
	}
	
	return view;
}

KYZTextEditorIface *KYZisFactory::createTextEditorIface()
{
	KLibFactory *factory = KLibLoader::self()->factory("libkyzispart");
	if (!factory) {
		kdDebug() << "Kyzis::createBuffer() called with no factory, discarding" << endl;
		return 0;
	}
	
	QObject *obj = factory->create(Kyzis::me, "kyzispart", "KParts::ReadWritePart");
	return dynamic_cast<KYZTextEditorIface*>(obj);
}

YZBuffer *KYZisFactory::doCreateBuffer() {
	return new KYZisBuffer;
}

void KYZisFactory::popupMessage( const QString& message ) {
	KYZisView *v = static_cast<KYZisView*>(currentView());
	KMessageBox::information(v, message, _( "Error" ));
}

void KYZisFactory::closeView() {
	if (Kyzis::me) {
		Kyzis::me->closeWindow( lastMdi );
	}
	else if (kapp->name() == QString::fromLatin1("kdevelop") ) {
		yzDebug() << "Calling kdevelop" <<endl;
		DCOPClient *client = kapp->dcopClient();
		QByteArray data;
		QDataStream arg(data, IO_WriteOnly);
		arg << QCString("file_close");
		bool w = client->send(client->appId(), "MainWindow", "activateAction(QCString)", data);
		if (w) {
			yzDebug() << "DCOP call successful for " << client->appId() << " to close view in kdevelop" << endl;
		} else {
			yzDebug() << "DCOP call failed for " << client->appId() << endl;
			popupMessage( "DCOP communication is broken !" );
		}
			
	}
	lastMdi = 0;
}

void KYZisFactory::doDeleteView( YZView *view ) {
	KYZisView *kview = dynamic_cast<KYZisView*>(view);
	if ( kview && Kyzis::me ) {
		lastMdi = kview->getMdiChildView();
		QTimer::singleShot(0, this, SLOT( closeView() ));
	}
}

void KYZisFactory::deleteBuffer(YZBuffer* b) {
	delete b;
}

void KYZisFactory::setFocusMainWindow() {
	KYZisView *yv = static_cast<KYZisView*>( currentView() );
	yv->setFocusMainWindow();
}

void KYZisFactory::setFocusCommandLine() {
	KYZisView *yv = static_cast<KYZisView*>( currentView() );
	yv->setFocusCommandLine();
}

bool KYZisFactory::promptYesNo(const QString& title, const QString& message) {
	int v = KMessageBox::questionYesNo(static_cast<KYZisView*>(currentView()), message, title);
	if (v == KMessageBox::Yes) return true;
	else return false;
}

int KYZisFactory::promptYesNoCancel(const QString& title, const QString& message) {
	int v = KMessageBox::questionYesNoCancel(static_cast<KYZisView*>(currentView()), message, title);
	if (v == KMessageBox::Yes) return 0;
	else if (v == KMessageBox::No) return 1;
	return 2;
}

void KYZisFactory::splitHorizontally(YZView* /*view*/) {
}

#include "factory.moc"
