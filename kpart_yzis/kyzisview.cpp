/* This file is part of the Yzis libraries
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
 *  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kyzisview.h"
#include "kyzissession.h"
#include "kyziseditor.h"
#include "kyziscommandwidget.h"
#include "kyzisinfobar.h"
#include "kteview.h"

#include <QSignalMapper>
#include <QGridLayout>
#include <QPainter>
#include <QScrollBar>

#include <kactioncollection.h>
#include <kaction.h>

#include <libyzis/view.h>
#include <libyzis/buffer.h>
#include <libyzis/debug.h>

KYZisView::KYZisView(YZBuffer* buffer, QWidget* parent)
	: QWidget(parent), YZView(buffer, KYZisSession::self(), 0, 0),
	actionCollection(0), signalMapper(0), m_painter(0)
{
	m_editor = new KYZisEditor( this );
	m_editor->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
	// TODO: remove this, as soon as we can configure the edior component
	m_editor->setPalette( Qt::white, Qt::black, 0 );

	m_command = new KYZisCommand( this );

	mVScroll = new QScrollBar( this );
	connect( mVScroll, SIGNAL( sliderMoved(int) ), this, SLOT( scrollView( int ) ) );
	connect( mVScroll, SIGNAL( prevLine() ), this, SLOT( scrollLineUp() ) );
	connect( mVScroll, SIGNAL( nextLine() ), this, SLOT( scrollLineDown() ) );
	mVScroll->setMaximum( buffer->lineCount() - 1 );

	m_infoBar = new KYZisInfoBar( this );
	m_infoBar->setMode( mode() );
	guiSyncViewInfo();

	QGridLayout* g = new QGridLayout( this );
	g->setMargin( 0 );
	g->setSpacing( 0 );
	g->addWidget( m_editor, 0, 0 );
	g->addWidget( mVScroll, 0, 1 );
	g->addWidget( m_command, 1, 0 );
	g->addWidget( m_infoBar, 2, 0 );


	initKeys();

	m_editor->setFocus();
	setFocusProxy( m_editor );
}

KYZisView::~KYZisView()
{
	delete signalMapper;
	for( int i = actionCollection->count() - 1; i>= 0; --i )
		delete actionCollection->takeAction( actionCollection->action(i) );
	delete actionCollection;
}

void KYZisView::setFocusMainWindow()
{
	m_editor->setFocus();
}

void KYZisView::setFocusCommandLine()
{
	m_command->setFocus();
}

void KYZisView::guiScroll(int dx, int dy)
{
	m_editor->scroll( dx, dy );
}

QString KYZisView::guiGetCommandLineText() const 
{
	return m_command->text();
}

void KYZisView::guiSetCommandLineText( const QString& text )
{
	m_command->setText( text );
}

void KYZisView::guiDisplayInfo(const QString& info) 
{
        //m_central->setText(info);
        //clean the info 2 seconds later
	//QTimer::singleShot(2000, this, SLOT( resetInfo() ) );
}

void KYZisView::guiSyncViewInfo() 
{
	m_editor->setCursor( viewCursor().screenX(), viewCursor().screenY() );
	m_infoBar->setLineInfo( getLineStatusString() );

	QString fileInfo;
	fileInfo +=( myBuffer()->fileIsNew() )?"N":" ";
	fileInfo +=( myBuffer()->fileIsModified() )?"M":" ";
	m_infoBar->setFileInfo( fileInfo );

	m_infoBar->setFileName( myBuffer()->fileName() );

//	if (mVScroll->value() != (int)getCurrentTop() && !mVScroll->draggingSlider())
//		mVScroll->setValue( getCurrentTop() );
//	emit cursorPositionChanged();
	guiModeChanged();
}

void KYZisView::guiModeChanged()
{
	m_editor->updateCursor();
	m_infoBar->setMode( mode() );
}

bool KYZisView::guiPopupFileSaveAs() 
{
       return false;
}

void KYZisView::guiFilenameChanged()
{

}

void KYZisView::guiHighlightingChanged()
{

}

void KYZisView::guiNotifyContentChanged(const YZSelection& s)
{
        YZSelectionMap m = s.map();
        // convert each interval to QWidget coordinates and update
        for( int i = 0; i < m.size(); ++i ) {
                YZInterval interval = m[i] - getScreenPosition();
                QRect r;
                if ( interval.fromPos().y() == interval.toPos().y() ) {
                        r = interval.boundingRect();
                        r.setBottom( r.bottom() + 1 );
                        r.setRight( r.right() + 1 );
                } else {
                        // XXX optimise : split into multiple qrect
                        r.setTop( interval.fromPos().y() );
                        r.setBottom( interval.toPos().y() + 1 );
                        r.setLeft( 0 );
                        r.setRight( getColumnsVisible() );
                }
//              dbg() << "notifiyContentChanged: interval=" << interval.fromPos() << "," << interval.toPos()
//                                      << ", r=" << r.topLeft() << "," << r.bottomRight();
                r.setBottomRight( m_editor->translatePositionToReal( r.bottomRight() ) );
                r.setTopLeft( m_editor->translatePositionToReal( r.topLeft() ) );
//              dbg() << " => " << r.topLeft() << "," << r.bottomRight() << endl;
                m_editor->update( r );
        }
}

void KYZisView::guiPreparePaintEvent(int min_y, int max_y)
{
	yzDebug() << "KYZisView::guiPreparePaintEvent" << endl;
	m_painter = new QPainter( m_editor );
	m_drawBuffer.setCallbackArgument( m_painter );
}

void KYZisView::guiPaintEvent( const YZSelection& drawMap ) {
	YZView::guiPaintEvent( drawMap );
}

void KYZisView::guiEndPaintEvent()
{
	delete m_painter;
	yzDebug() << "KYZisView::endPaintEvent" << endl;
}

void KYZisView::guiDrawCell(int x, int y, const YZDrawCell& cell, void* arg)
{
	m_editor->guiDrawCell( x, y, cell, (QPainter*)arg );
}

void KYZisView::guiDrawClearToEOL(int x, int y, const QChar& clearChar)
{
	m_editor->guiDrawClearToEOL( x, y, clearChar, m_painter );
}

void KYZisView::guiDrawSetLineNumber(int, int, int)
{

}

void KYZisView::guiDrawSetMaxLineNumber( int max )
{
}

const QString& KYZisView::convertKey( int key ) {
	return keys[ key ];
}

void KYZisView::initKeys() {
	keys[ Qt::Key_Escape ] = "<ESC>" ;
	keys[ Qt::Key_Tab ] = "<TAB>" ;
	keys[ Qt::Key_Backtab ] = "<BTAB>" ;
	keys[ Qt::Key_Backspace ] = "<BS>" ;
	keys[ Qt::Key_Return ] = "<ENTER>" ;
	keys[ Qt::Key_Enter ] = "<ENTER>" ;
	keys[ Qt::Key_Insert ] = "<INS>" ;
	keys[ Qt::Key_Delete ] = "<DEL>" ;
	keys[ Qt::Key_Pause ] = "<PAUSE>" ;
	keys[ Qt::Key_Print ] = "<PRINT>" ;
	keys[ Qt::Key_SysReq ] = "<SYSREQ>" ;
	keys[ Qt::Key_Home ] = "<HOME>" ;
	keys[ Qt::Key_End ] = "<END>" ;
	keys[ Qt::Key_Left ] = "<LEFT>" ;
	keys[ Qt::Key_Up ] = "<UP>" ;
	keys[ Qt::Key_Right ] = "<RIGHT>" ;
	keys[ Qt::Key_Down ] = "<DOWN>" ;
	//keys[ Qt::Key_Prior ] = "<PUP>" ; seems like it doesn't exist any longer with qt 4.3
	//keys[ Qt::Key_Next ] = "<PDOWN>" ; seems like it doesn't exist any longer with qt 4.3
	keys[ Qt::Key_PageUp ] = "<PUP>" ;
	keys[ Qt::Key_PageDown ] = "<PDOWN>" ;
	keys[ Qt::Key_Shift ] = "<SHIFT>" ;
	keys[ Qt::Key_Control ] = "<CTRL>" ;
	keys[ Qt::Key_Meta ] = "<META>" ;
	keys[ Qt::Key_Alt ] = "<ALT>" ;
//hmm ignore it	keys[ Qt::Key_CapsLock ] = "<CAPSLOCK>" ;
//hmm ignore it	keys[ Qt::Key_NumLock ] = "<NUMLOCK>" ;
//hmm ignore it	keys[ Qt::Key_ScrollLock ] = "<SCROLLLOCK>" ;
	keys[ Qt::Key_Clear ] = "<CLEAR>" ;
	keys[ Qt::Key_F1 ] = "<F1>" ;
	keys[ Qt::Key_F2 ] = "<F2>" ;
	keys[ Qt::Key_F3 ] = "<F3>" ;
	keys[ Qt::Key_F4 ] = "<F4>" ;
	keys[ Qt::Key_F5 ] = "<F5>" ;
	keys[ Qt::Key_F6 ] = "<F6>" ;
	keys[ Qt::Key_F7 ] = "<F7>" ;
	keys[ Qt::Key_F8 ] = "<F8>" ;
	keys[ Qt::Key_F9 ] = "<F9>" ;
	keys[ Qt::Key_F10 ] = "<F10>" ;
	keys[ Qt::Key_F11 ] = "<F11>" ;
	keys[ Qt::Key_F12 ] = "<F12>" ;
	keys[ Qt::Key_F13 ] = "<F13>" ;
	keys[ Qt::Key_F14 ] = "<F14>" ;
	keys[ Qt::Key_F15 ] = "<F15>" ;
	keys[ Qt::Key_F16 ] = "<F16>" ;
	keys[ Qt::Key_F17 ] = "<F17>" ;
	keys[ Qt::Key_F18 ] = "<F18>" ;
	keys[ Qt::Key_F19 ] = "<F19>" ;
	keys[ Qt::Key_F20 ] = "<F20>" ;
	keys[ Qt::Key_F21 ] = "<F21>" ;
	keys[ Qt::Key_F22 ] = "<F22>" ;
	keys[ Qt::Key_F23 ] = "<F23>" ;
	keys[ Qt::Key_F24 ] = "<F24>" ;
	keys[ Qt::Key_F25 ] = "<F25>" ;
	keys[ Qt::Key_F26 ] = "<F26>" ;
	keys[ Qt::Key_F27 ] = "<F27>" ;
	keys[ Qt::Key_F28 ] = "<F28>" ;
	keys[ Qt::Key_F29 ] = "<F29>" ;
	keys[ Qt::Key_F30 ] = "<F30>" ;
	keys[ Qt::Key_F31 ] = "<F31>" ;
	keys[ Qt::Key_F32 ] = "<F32>" ;
	keys[ Qt::Key_F33 ] = "<F33>" ;
	keys[ Qt::Key_F34 ] = "<F34>" ;
	keys[ Qt::Key_F35 ] = "<F35>" ;
//	keys[ Qt::Key_BracketLeft ] = "[";
//	keys[ Qt::Key_BracketRight ] = "]";


	actionCollection = new KActionCollection( this );
	signalMapper = new QSignalMapper( this );
	connect( signalMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( sendMultipleKeys( const QString& ) ) );
}

YZDrawCell KYZisView::getCursorDrawCell( )
{
	return m_drawBuffer.at( getCursor() - YZCursor(getDrawCurrentLeft(), getDrawCurrentTop( )) );
}

void KYZisView::registerModifierKeys( const QString& keys ) {
	KAction* k = new KAction( actionCollection );
	k->setShortcut( KShortcut( keysToShortcut( keys ) ) );
	connect( k, SIGNAL( triggered() ), signalMapper, SLOT( map() ) );
		
	signalMapper->setMapping( k, keys );
}
void KYZisView::unregisterModifierKeys( const QString& keys ) {
	QByteArray ke = keys.toUtf8();
	QAction* q = actionCollection->action( ke.data() );
	if ( q == NULL ) {
		yzDebug() << "No KAction for " << keys << endl;
		return;
	}
	actionCollection->takeAction( q );
#warning port to KDE4 API
#if 0
	KAccel* accel = actionCollection->kaccel();
	if ( accel ) {
		accel->remove( keys );
		accel->updateConnections();
	}
#endif
	signalMapper->removeMappings( q );
	delete q;
}

QString KYZisView::keysToShortcut( const QString& keys ) {
	QString ret = keys;
	ret = ret.replace( "<CTRL>", "CTRL+" );
	ret = ret.replace( "<SHIFT>", "SHIFT+" );
	ret = ret.replace( "<ALT>", "ALT+" );
	return ret;
}

void KYZisView::scrollLineUp() {
	scrollView( getCurrentTop() - 1 );
}

void KYZisView::scrollLineDown() {
	scrollView( getCurrentTop() + 1 );
}


// scrolls the _view_ on a buffer and moves the cursor it scrolls off the screen
void KYZisView::scrollView( int value ) {
	if ( value < 0 )
		value = 0;
	else if ( value > myBuffer()->lineCount() - 1 )
		value = myBuffer()->lineCount() - 1;

	// only redraw if the view actually moves
	if (value != getCurrentTop()) {
		alignViewBufferVertically( value );

		if ( !mVScroll->isSliderDown() )
			mVScroll->setValue( value );
	}
}

void KYZisView::sendMultipleKeys( const QString& k )
{
	KYZisSession::self()->sendMultipleKeys( this, k );
}

#include "kyzisview.moc"