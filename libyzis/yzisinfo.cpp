/* This file is part of the Yzis libraries
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

/* System */
#include <iostream>

/* Qt */
#include <qfileinfo.h>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>

/* yzis */
#include "debug.h"
#include "history.h"
#include "internal_options.h"
#include "mode_ex.h"
#include "buffer.h"
#include "mode_search.h"
#include "session.h"
#include "view.h"
#include "yzisinfo.h"
#include "yzisinfojumplistrecord.h"
#include "yzisinfostartpositionrecord.h"

using namespace std;

class YZYzisinfoCursor;

/**
 * Constructor
 */

YZYzisinfo::YZYzisinfo() {
	
	QString path = QDir::homePath() + "/.yzis";
	mYzisinfo.setFileName( path + "/yzisinfo" );

	mYzisinfoInitialized = false;
}

/**
 * Constructor
 */

YZYzisinfo::YZYzisinfo( const QString & path ) {
	
	mYzisinfo.setFileName( path );
}

/**
 * Destructor
 */

YZYzisinfo::~YZYzisinfo() {
}

/**
 * YZYsisinfo::readYzisinfo
 */

void YZYzisinfo::readYzisinfo() {
	
	if ( mYzisinfoInitialized ) {
		return;
	}

	mYzisinfoInitialized = true;
	
	if ( mYzisinfo.open( QIODevice::ReadOnly ) ) {
		QTextStream stream( &mYzisinfo );
		QString line;
		
		while ( !stream.atEnd() ) {
			line = stream.readLine(); // line of text excluding '\n'
			line = line.trimmed();
			
			// Ignore empty lines
			
			if ( line.isEmpty() ) {
				continue;
			}
			
			// Ignore comment lines

			if ( line.startsWith("#") ) {
				continue;
			}
			
			// If we have got this far then we must have a line that is 
			// a) Not a comment
			// b) Not empty
			// So split it into a list using whitespace as the separator
			
			QStringList list = line.split( QRegExp( "\\s" ));
			
			if ( list[0] == "hlsearch" ) {
				if ( list[1] == "on" ) {
					bool on = true;
					YZSession::self()->getOptions()->setOptionFromString( &on, "hlsearch" );
				} else {
					bool on = false; 
					YZSession::self()->getOptions()->setOptionFromString( &on, "hlsearch" );
				}
			}
			
			if ( list[0].startsWith(":") || list[0] == "command_list" ) {
				YZModeEx *ex = YZSession::self()->getExPool();
				YZHistory *history = ex->getHistory();
				
				history->addEntry( (list.join(" ")).remove(0, 1) );
			}
			
			if ( list[0].startsWith("?") || list[0] == "search_list" ) {
				YZModeSearch *search = dynamic_cast<YZModeSearch*>(YZSession::self()->getModes()[ YZMode::MODE_SEARCH ]);
				YZHistory *history = search->getHistory();
				
				history->addEntry( (list.join(" ")).remove(0, 1) );
			}
			
			if ( list[0].startsWith(">") || list[0] == "start_position" ) {
				mStartPosition.push_back( new YZYzisinfoStartPositionRecord( list[3], list[1].toInt(), list[2].toInt() ) );
			}
			
			if ( list[0].startsWith("_") || list[0] == "search_history" ) {
				mJumpList.push_back( new YZYzisinfoJumpListRecord( list[3], list[1].toInt(), list[2].toInt() ) );
				mCurrentJumpListItem++;
			}
			
			if ( list[0].startsWith("\"") ) {
				QChar key = list[0].at(1);
				
				QString line;
				QStringList contents;
				int length = list[3].toInt();
				
				if ( list[1] == "CHAR" ) {
					for ( int i = 0; i < length; ++i ) {
						contents << stream.readLine();
					}
				} else {
					contents << QString::null;
				
					for ( int i = 0; i < length; ++i ) {
						contents << stream.readLine();
					}
				
					contents << QString::null;
				}
				
				yzDebug() << "Key:<" << key.toAscii() << ">" << endl;
				yzDebug() << "Length:<" << contents.size() << ">" << endl;
				for ( int i = 0; i < contents.size(); ++i ) {
					yzDebug() << "<" << contents.at(i) << ">" << endl;
				}
				
				YZSession::self()->setRegister( key, contents );	
			}
		}
		
		mYzisinfo.close();
	} else {
		yzDebug() << "Unable to open file " << mYzisinfo.fileName() << endl;
	}
}

/**
 * YZYzisinfo::updateStartPosition
 */
 
void YZYzisinfo::updateStartPosition( const YZBuffer *buffer, const int x, const int y ) {
	bool found = false;

	for ( StartPositionVector::Iterator it = mStartPosition.begin(); it != mStartPosition.end(); ++it ) {
		if ( (*it)->filename() == buffer->fileName() ) {
			found = true;
			mStartPosition.erase(it);
			mStartPosition.push_back( new YZYzisinfoStartPositionRecord( buffer->fileName(), x, y ) );
			return;
		}
	}
	         
	if ( ! found ) {         
		mStartPosition.push_back( new YZYzisinfoStartPositionRecord( buffer->fileName(), x, y ) );
	}
	
	return;
}

/**
 * YZYzisinfo::updateJumpList
 */
 
void YZYzisinfo::updateJumpList( const YZBuffer *buffer, const int x, const int y ) {
	bool found = false;

	for ( JumpListVector::Iterator it = mJumpList.begin(); it != mJumpList.end(); ++it ) {
		if ( (*it)->filename() == buffer->fileName() ) {
			if ( (*it)->position().x() == x && (*it)->position().y() == y ) {
				found = true;
				break;
			}
		}
	}
	         
	if ( ! found ) {         
		mJumpList.push_back( new YZYzisinfoJumpListRecord( buffer->fileName(), x, y ) );
	}
	
	return;
}

/**
 * YZYzisinfo::writeYzisinfo
 */
 
void YZYzisinfo::writeYzisinfo() {
	if ( mYzisinfo.open( QIODevice::WriteOnly ) ) {
		QTextStream write( &mYzisinfo );
		write.setCodec( QTextCodec::codecForName("utf8"));
		// Write header
		
		write << "# This yzisinfo file was generated by Yzis " << VERSION_CHAR << "." << endl;
		write << "# You may edit it if you're careful!" << endl;
		write << endl;
		
		// Write whether hlsearch is on or off
		
		write << "# Set hlsearch on or off:" << endl;
		write << "hlsearch ";
		if ( YZSession::self()->getBooleanOption( "hlsearch") ) {
			write << "on" << endl;
		} else {
			write << "off" << endl;
		}
		write << endl;
		
		write << "# Command Line History (oldest to newest):" << endl; 
		saveExHistory( write );
		write << endl;
		
		write << "# Search String History (oldest to newest):" << endl;		
		saveSearchHistory( write );
		write << endl;
		
		write << "# Position to start at when opening file (oldest to newest):" << endl;
		saveStartPosition( write );
		write << endl;
		
		write << "# Jump list (oldest to newest):" << endl;
		saveJumpList( write );
		write << endl;
		
		write << "# Registers:" << endl;
		saveRegistersList( write );
		write << endl;
		
		mYzisinfo.close();
	}
}

/**
 * YZYzisinfo::saveExHistory
 */
 
void YZYzisinfo::saveExHistory( QTextStream & write ) {
	YZHistory *history = YZSession::self()->getExPool()->getHistory();
	history->writeToStream( write );
}

/**
 * YZYzisinfo::saveSearchHistory
 */
 
void YZYzisinfo::saveSearchHistory( QTextStream & write ) {
	YZModeSearch *search = dynamic_cast<YZModeSearch*>(YZSession::self()->getModes()[ YZMode::MODE_SEARCH ] );
	YZHistory *history = search->getHistory();
	history->writeToStream( write );
}

/**
 * YZYzisinfo::saveStartPosition
 */
 
void YZYzisinfo::saveStartPosition( QTextStream & write ) {

	int start = 0;
	int end = mStartPosition.count();
	
	if ( end > 100 ) {
		start = end - 100;
	}

	for( int i = start; i < end; ++i ) {
		write << "> ";
		yzDebug() << (mStartPosition[i])->position()->x();
		write << (mStartPosition[i])->position()->x();
		write << " "; 
		yzDebug() << (mStartPosition[i])->position()->y();
		write << (mStartPosition[i])->position()->y();
		write << " ";
		yzDebug() << (mStartPosition[i])->filename() << endl;
		write << (mStartPosition[i])->filename() << endl;
	}
}	

/**
 * YZYsisinfo::saveJumpList
 */
 
void YZYzisinfo::saveJumpList( QTextStream & write ) {

	int start = 0;	
	int end = mJumpList.count();
	
	if ( end > 100 ) {
		start = end - 100;
	}
	
	for( int i = start; i < end; ++i ) {
		write << "_" << " ";
		write << mJumpList[i]->position().x();
		write << " ";
		write << mJumpList[i]->position().y();
		write << " ";
		write << mJumpList[i]->filename() << endl;
	}
}

/**
 * YZYzisinfo::saveRegisters
 */
 
void YZYzisinfo::saveRegistersList( QTextStream & write ) {

	QList<QChar> list = YZSession::self()->getRegisters();	
	
	for ( int i = 0; i < list.size(); ++i ) {
		QStringList contents = YZSession::self()->getRegister( list.at(i) );
		
		write << "\"" << list.at(i) << " ";
		
		if ( contents.size() >= 3 ) {
			write << "LINE  " << contents.size() - 2 << endl;
		} else {
			write << "CHAR  " << contents.size() << endl;
		}
		
		for ( int j = 0; j < contents.size(); ++j ) {
			if ( ( contents.at(j) ).isNull() ) {
				continue;
			}
			
			write << contents.at(j) << "\n";
		}
	}
}

/**
 * YZYzisinfo::startPosition
 */

YZCursor * YZYzisinfo::startPosition( const QString& filename ) const {

	for ( StartPositionVector::ConstIterator it = mStartPosition.begin(); it != mStartPosition.end(); ++it ) {
		if ( (*it)->filename() == filename ) {
			return (*it)->position();
		}
	}

	return 0;
}
YZCursor * YZYzisinfo::startPosition( const YZBuffer *buffer ) const {
	return startPosition( buffer->fileName() );
}

/**
 * YZYzisinfo::searchPosition
 */
 
YZCursor * YZYzisinfo::searchPosition( const YZBuffer */*buffer*/) {
	
	for ( JumpListVector::Iterator it = mJumpList.begin(); it != mJumpList.end(); ++it ) {
		/*if ( (*it)->filename() == buffer->fileName() ) {
			return (*it)->previousSearchPosition();
		}*/
	}
            
	return new YZCursor( YZSession::self()->currentView()->getBufferCursor() );
}

const YZCursor * YZYzisinfo::previousJumpPosition() {

	bool found = false;	
	bool repeating = false;

	while ( true ) {
		if ( mCurrentJumpListItem == 0 ) {
			// Make sure we don't end up in a endless loop
			if ( repeating ) {
				break;
			}
			
			repeating = true;
			mCurrentJumpListItem = mJumpList.count();
		}
		
		--mCurrentJumpListItem;
	
		if ( mJumpList[mCurrentJumpListItem]->filename() == YZSession::self()->currentView()->myBuffer()->fileName() ) {
			found = true;
			break;
		}
	}
	
	if ( found ) {
		return &mJumpList[mCurrentJumpListItem]->position();
	} else {
		return &YZSession::self()->currentView()->getCursor();
	}
}


/*
 * END OF FILE
 */
