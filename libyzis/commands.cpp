/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2003-2004 Philippe Fremy <phil@freehackers.org>
 *  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
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

#include <assert.h>
#include "commands.h"
#include "debug.h"
#include "view.h"
#include "viewcursor.h"
#include "session.h"
#include "buffer.h"
#include "cursor.h"
#include "linesearch.h"
#include "action.h"
#include "mark.h"
#if QT_VERSION < 0x040000
#include <qregexp.h>
#include <qptrlist.h>
#else
#include <QRegExp>
#endif

/**
 */
YZCommandPool::YZCommandPool() {
	commands.clear();
#if QT_VERSION < 0x040000
	commands.setAutoDelete(true);
#endif
}

YZCommandPool::~YZCommandPool() {
#if QT_VERSION >= 0x040000
	for ( int ab = 0 ; ab < commands.size(); ++ab)
		delete commands.at(ab);
#endif
	commands.clear();
}

/**
 * NORMAL MODE COMMANDS
 */
void YZCommandPool::initPool() {
//	textObjects << "aw" << "iw";
	commands.append( new YZNewMotion("0", &YZCommandPool::gotoSOL, ARG_NONE) );
	commands.append( new YZNewMotion("$", &YZCommandPool::gotoEOL, ARG_NONE) );
	commands.append( new YZNewMotion("^", &YZCommandPool::firstNonBlank, ARG_NONE) );
	commands.append( new YZNewMotion("w", &YZCommandPool::moveWordForward, ARG_NONE) );
	commands.append( new YZNewMotion("b", &YZCommandPool::moveWordBackward, ARG_NONE) );
	commands.append( new YZNewMotion("j", &YZCommandPool::moveDown, ARG_NONE) );
	commands.append( new YZNewMotion("k", &YZCommandPool::moveUp, ARG_NONE) );
	commands.append( new YZNewMotion("h", &YZCommandPool::moveLeft, ARG_NONE) );
	commands.append( new YZNewMotion("l", &YZCommandPool::moveRight, ARG_NONE) );
	commands.append( new YZNewMotion("<BS>", &YZCommandPool::moveLeftWrap, ARG_NONE) );
	commands.append( new YZNewMotion(" ", &YZCommandPool::moveRightWrap, ARG_NONE) );
	commands.append( new YZNewMotion("f", &YZCommandPool::findNext, ARG_CHAR) );
	commands.append( new YZNewMotion("t", &YZCommandPool::findBeforeNext, ARG_CHAR) );
	commands.append( new YZNewMotion("F", &YZCommandPool::findPrevious, ARG_CHAR) );
	commands.append( new YZNewMotion("T", &YZCommandPool::findAfterPrevious, ARG_CHAR) );
	commands.append( new YZNewMotion(";", &YZCommandPool::repeatFind, ARG_CHAR) );
	commands.append( new YZNewMotion("*", &YZCommandPool::searchWord, ARG_NONE) );
	commands.append( new YZNewMotion("g*", &YZCommandPool::searchWord, ARG_NONE) );
	commands.append( new YZNewMotion("#", &YZCommandPool::searchWord, ARG_NONE) );
	commands.append( new YZNewMotion("g#", &YZCommandPool::searchWord, ARG_NONE) );
	commands.append( new YZNewMotion("n", &YZCommandPool::searchNext, ARG_NONE) );
	commands.append( new YZNewMotion("N", &YZCommandPool::searchPrev, ARG_NONE) );
	commands.append( new YZNewMotion("<HOME>", &YZCommandPool::gotoSOL, ARG_NONE) );
	commands.append( new YZNewMotion("<END>", &YZCommandPool::gotoEOL, ARG_NONE) );
	commands.append( new YZNewMotion("<LEFT>", &YZCommandPool::moveLeft, ARG_NONE) );
	commands.append( new YZNewMotion("<RIGHT>", &YZCommandPool::moveRight, ARG_NONE) );
	commands.append( new YZNewMotion("<UP>", &YZCommandPool::moveUp, ARG_NONE) );
	commands.append( new YZNewMotion("<DOWN>", &YZCommandPool::moveDown, ARG_NONE) );
	commands.append( new YZNewMotion("<PUP>", &YZCommandPool::movePageUp, ARG_NONE) );
	commands.append( new YZNewMotion("<PDOWN>", &YZCommandPool::movePageDown, ARG_NONE) );
	commands.append( new YZNewMotion("%", &YZCommandPool::matchPair, ARG_NONE) );
	commands.append( new YZNewMotion("`", &YZCommandPool::gotoMark, ARG_MARK) );
	commands.append( new YZNewMotion("'", &YZCommandPool::gotoMark, ARG_MARK) );
	commands.append( new YZNewMotion("<ENTER>", &YZCommandPool::firstNonBlankNextLine, ARG_NONE) );
	commands.append( new YZNewMotion("gg", &YZCommandPool::gotoLine, ARG_NONE) );
	commands.append( new YZNewMotion("G", &YZCommandPool::gotoLine, ARG_NONE) );
	commands.append( new YZCommand("I", &YZCommandPool::insertAtSOL) );
	commands.append( new YZCommand("i", &YZCommandPool::gotoInsertMode) );
	commands.append( new YZCommand("<INS>", &YZCommandPool::gotoInsertMode) );
	commands.append( new YZCommand(":", &YZCommandPool::gotoExMode) );
	commands.append( new YZCommand("R", &YZCommandPool::gotoReplaceMode) );
	commands.append( new YZCommand("v", &YZCommandPool::gotoVisualMode) );
	commands.append( new YZCommand("V", &YZCommandPool::gotoVisualLineMode) );
	commands.append( new YZCommand("z<ENTER>", &YZCommandPool::gotoLineAtTop) );
	commands.append( new YZCommand("z+", &YZCommandPool::gotoLineAtTop) );
	commands.append( new YZCommand("z.", &YZCommandPool::gotoLineAtCenter) );
	commands.append( new YZCommand("z-", &YZCommandPool::gotoLineAtBottom) );
	commands.append( new YZCommand("dd", &YZCommandPool::deleteLine) );
	commands.append( new YZCommand("d", &YZCommandPool::del, ARG_MOTION) );
	commands.append( new YZCommand("D", &YZCommandPool::deleteToEOL) );
	commands.append( new YZCommand("x", &YZCommandPool::deleteChar) );
	commands.append( new YZCommand("yy", &YZCommandPool::yankLine) );
	commands.append( new YZCommand("y", &YZCommandPool::yank, ARG_MOTION) );
	commands.append( new YZCommand("Y", &YZCommandPool::yankToEOL) );
	commands.append( new YZCommand("cc", &YZCommandPool::changeLine) );
	commands.append( new YZCommand("c", &YZCommandPool::change, ARG_MOTION) );
	commands.append( new YZCommand("C", &YZCommandPool::changeToEOL) );
	commands.append( new YZCommand("p", &YZCommandPool::pasteAfter) );
	commands.append( new YZCommand("P", &YZCommandPool::pasteBefore) );
	commands.append( new YZCommand("o", &YZCommandPool::insertLineAfter) );
	commands.append( new YZCommand("O", &YZCommandPool::insertLineBefore) );
	commands.append( new YZCommand("a", &YZCommandPool::append) );
	commands.append( new YZCommand("A", &YZCommandPool::appendAtEOL) );
	commands.append( new YZCommand("J", &YZCommandPool::joinLine) );
	commands.append( new YZCommand("<", &YZCommandPool::indent, ARG_MOTION ) );
	commands.append( new YZCommand("<<", &YZCommandPool::indent ) );
	commands.append( new YZCommand(">", &YZCommandPool::indent, ARG_MOTION ) );
	commands.append( new YZCommand(">>", &YZCommandPool::indent ) );
	commands.append( new YZCommand("ZZ", &YZCommandPool::saveAndClose) );
	commands.append( new YZCommand("ZQ", &YZCommandPool::closeWithoutSaving) );
	commands.append( new YZCommand("/", &YZCommandPool::searchForwards) );
	commands.append( new YZCommand("?", &YZCommandPool::searchBackwards) );
	commands.append( new YZCommand("~", &YZCommandPool::changeCase) );
	commands.append( new YZCommand("m", &YZCommandPool::mark, ARG_CHAR) );
	commands.append( new YZCommand("r", &YZCommandPool::replace, ARG_CHAR) );
	commands.append( new YZCommand("u", &YZCommandPool::undo) );
	commands.append( new YZCommand("U", &YZCommandPool::redo) );
	commands.append( new YZCommand("<CTRL>r", &YZCommandPool::redo) );
	commands.append( new YZCommand("<CTRL>R", &YZCommandPool::redo) );
	commands.append( new YZCommand("q", &YZCommandPool::macro) );
	commands.append( new YZCommand("@", &YZCommandPool::replayMacro) );
	commands.append( new YZCommand("<CTRL>l", &YZCommandPool::redisplay) );
	commands.append( new YZCommand("<CTRL>[", &YZCommandPool::gotoCommandMode) );
	commands.append( new YZCommand("<ESC>", &YZCommandPool::abort) );
	commands.append( new YZCommand("<DEL>", &YZCommandPool::delkey) );
	commands.append( new YZCommand("<ALT>:", &YZCommandPool::gotoExMode) );
}

cmd_state YZCommandPool::execCommand(YZView *view, const QString& inputs) {
//	yzDebug() << "ExecCommand : " << inputs << endl;
	unsigned int count=1;
	bool hadCount = false;
	unsigned int i=0;
#if QT_VERSION < 0x040000
	QValueList<QChar> regs;
	YZView* it;
#else
	QList<QChar> regs;
#endif

	// read in the register operations and the counts
	while(i<inputs.length()) {
		if(inputs.at( i ).digitValue() > 0) {
			unsigned int j=i+1;
			while(j<inputs.length() && inputs.at(j).digitValue() >= 0)
				j++;
			count*=inputs.mid(i, j-i).toInt();
			i=j;
			yzDebug() << "Count " << count << endl;
			hadCount=true; //we found digits given by the user
		} else if(inputs.at( i ) == '\"') {
			if(++i>=inputs.length())
				break;
			regs << inputs.at(i++);
		} else
			break;
	}
	//if regs is empty add the default register
	if ( regs.count() == 0 )
		regs << '\"';

	if(i>=inputs.length())
		return NO_COMMAND_YET;

	// collect all the commands
#if QT_VERSION < 0x040000
	QPtrList<const YZCommand> cmds, prevcmds;
	cmds.setAutoDelete(false);
	prevcmds.setAutoDelete(false);
#else
	QList<YZCommand*> cmds, prevcmds;
#endif

	unsigned int j=i;

	// retrieve all the matching commands
	// .. first the ones whose first key matches
	if(j<inputs.length()) {
#if QT_VERSION < 0x040000
		for(commands.first(); commands.current(); commands.next())
			if(commands.current()->keySeq().startsWith(inputs.mid(j,1)))
				cmds.append(commands.current());
#else
		for (int ab = 0; ab < commands.size(); ++ab )
			if (commands.at(ab)->keySeq().startsWith(inputs.mid(j,1)))
				cmds.append(commands.at(ab));
#endif
	}
	j++;
	// .. then the ones whose next keys match, too
	while(!cmds.isEmpty() && ++j<=inputs.length()) {
		prevcmds=cmds;
#if QT_VERSION < 0x040000
		// delete all the commands that don't match
		for(cmds.first(); cmds.current();)
			if(cmds.current()->keySeq().startsWith(inputs.mid(i,j-i)))
				cmds.next();
			else
				cmds.remove();
#else
		// delete all the commands that don't match
		for ( int bc = 0 ; bc < cmds.size() ;  )
			if(cmds.at(bc)->keySeq().startsWith(inputs.mid(i,j-i)))
				++bc;
			else
				cmds.removeAt(bc);
#endif
	}
	if(cmds.isEmpty()) {
		// perhaps it is a command with an argument, isolate all those
#if QT_VERSION < 0x040000
		for(prevcmds.first(); prevcmds.current();)
			if(prevcmds.current()->arg() == ARG_NONE)
				prevcmds.remove();
			else
				prevcmds.next();
#else
		for ( int bc = 0 ; bc < prevcmds.size() ; )
			if ( prevcmds.at(bc)->arg() == ARG_NONE )
				prevcmds.removeAt(bc);
			else
				++bc;
#endif
		if(prevcmds.isEmpty())
			return CMD_ERROR;
		// it really is a command with an argument, read it in
		const YZCommand *c=prevcmds.first();
		i=j-1;
		// read in a count that may follow
		if (c->arg() == ARG_CHAR) {//dont try to read a motion !
			(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, hadCount, inputs.mid(i)));
			return CMD_OK;
		}
		if(inputs.at(i).digitValue() > 0) {
			while(j<inputs.length() && inputs.at(j).digitValue() > 0)
				j++;
			count*=inputs.mid(i,j-i).toInt();
			i=j;
			if(i>=inputs.length() )
				return OPERATOR_PENDING;
		}

		QString s=inputs.mid(i);
		switch(c->arg()) {
		case ARG_MOTION:
			if(s[0]=='a' || s[0]=='i') {
				// text object
				if(s.length()==1)
					return OPERATOR_PENDING;
				else if(!textObjects.contains(s))
					return CMD_ERROR;
			} else {
#if QT_VERSION < 0x040000
				// motion, look for a motion that matches exactly
				for(commands.first(); commands.current(); commands.next()) {
					const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(commands.current());
					if(m && m->matches(s))
						break;
				}
#else
				bool matched = false;
				for (int ab = 0; ab < commands.size(); ++ab) {
					const YZNewMotion *m = dynamic_cast<const YZNewMotion*>(commands.at(ab));
					if (m && m->matches(s)) {
						matched = true;
						break;
					}
				}
#endif
#if QT_VERSION < 0x040000
				if(!commands.current()) {
					// look for an incomplete motion
					for(commands.first(); commands.current(); commands.next()) {
						const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(commands.current());
						if(m && m->matches(s, false))
							return OPERATOR_PENDING;
					}
					return CMD_ERROR;
				}
#else
				if (!matched) {
					for (int ab = 0; ab < commands.size(); ++ab ) {
						const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(commands.at(ab));
						if(m && m->matches(s, false))
							return OPERATOR_PENDING;

					}
				}
#endif
			}
			break;
		case ARG_CHAR:
		case ARG_REG:
			if(s.length()!=1)
				return CMD_ERROR;
			break;
		case ARG_MARK:
			if(s.length()!=1 || !YZCommand::isMark(s[0]))
				return CMD_ERROR;
			break;
		default:
			break;
		}
		// the argument is OK, go for it

#if QT_VERSION < 0x040000
		for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
			it->setPaintAutoCommit( false );
#else
		for ( int bc = 0; bc < view->myBuffer()->views().size(); ++bc )
			view->myBuffer()->views().at(bc)->setPaintAutoCommit(false);
#endif

		(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, hadCount, s));

#if QT_VERSION < 0x040000
		for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
			it->commitPaintEvent();
#else
		for ( int bc = 0; bc < view->myBuffer()->views().size(); ++bc )
			view->myBuffer()->views().at(bc)->commitPaintEvent();
#endif

	} else {
		// keep the commands that match exactly
		QString s=inputs.mid(i);
#if QT_VERSION < 0x040000
		for(cmds.first();cmds.current();) {
			if(cmds.current()->keySeq()!=s)
				cmds.remove();
			else
				cmds.next();
		}
#else
		for ( int ab = 0 ; ab < cmds.size(); ) {
			if (cmds.at(ab)->keySeq()!=s)
				cmds.removeAt(ab);
			else
				++ab;
		}
#endif
		if(cmds.isEmpty())
			return NO_COMMAND_YET;
		bool visual = view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL
		              || view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL_LINE;
		YZSelectionMap m;
		if(visual)
			m=view->getVisualSelection();
		const YZCommand *c=0;
		if(cmds.count()==1) {
			c=cmds.first();
			if(c->arg() == ARG_NONE || visual && c->arg() == ARG_MOTION)
				(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, hadCount, m));
			else
				return OPERATOR_PENDING;
		} else {
			/* two or more commands with the same name, we assert that these are exactly
			a cmd that needs a motion and one without an argument. In visual mode, we take
			the operator, in normal mode, we take the other. */
			//this is not sufficient, see the 'q' (record macro command), we need a q+ARG_CHAR and a 'q' commands //mm //FIXME
#if QT_VERSION < 0x040000
			for(cmds.first();cmds.current();cmds.next()) {
				if(cmds.current()->arg() == ARG_MOTION && visual ||
				        cmds.current()->arg() == ARG_NONE && !visual)
					c=cmds.current();
			}
#else
			for ( int ab = 0 ; ab < cmds.size(); ++ab ) {
				if ( cmds.at(ab)->arg() == ARG_MOTION && visual ||
						cmds.at(ab)->arg() == ARG_NONE && !visual )
					c = cmds.at(ab);
			}
#endif
			if(!c)
				return CMD_ERROR;
#if QT_VERSION < 0x040000
			for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
				it->setPaintAutoCommit( false );
#else
			for ( int bc = 0; bc < view->myBuffer()->views().size(); ++bc )
				view->myBuffer()->views().at(bc)->setPaintAutoCommit(false);
#endif
			if(visual)
				(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, hadCount, m));
			else
				(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, hadCount, QString::null));
#if QT_VERSION < 0x040000
			for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
				it->commitPaintEvent();
#else
			for ( int bc = 0; bc < view->myBuffer()->views().size(); ++bc )
				view->myBuffer()->views().at(bc)->commitPaintEvent();
#endif
		}
	}

	return CMD_OK;
}

bool YZNewMotion::matches(const QString &s, bool fully) const {
	QString ks=mKeySeq;
	if(s.startsWith(ks)) {
		switch(mArg) {
		case ARG_NONE:
			if(s.length() == ks.length())
				return true;
			break;
		case ARG_CHAR:
			if(s.length() == ks.length()+1 || !fully && s.length() == ks.length())
				return true;
			break;
		case ARG_MARK:
			if(s.length() == ks.length()+1 && isMark(s.at(s.length()-1)) || !fully && s.length() == ks.length())
				return true;
			break;
		default:
			break;
		}
	} else if(!fully && ks.startsWith(s))
		return true;

	return false;
}

YZCursor YZCommandPool::move(YZView *view, const QString &inputs, unsigned int count, bool usercount) {
#if QT_VERSION < 0x040000
	for(commands.first(); commands.current(); commands.next()) {
		// is the command a motion and does it match to the string?
		const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(commands.current());
		if(m && m->matches(inputs)) {
			// execute the corresponding method
			YZCursor to=(this->*(m->motionMethod()))(YZNewMotionArgs(view, count, inputs.right( m->keySeq().length()), 
					inputs.left(m->keySeq().length()), usercount ));
			return to;
		}
	}
#else
	for (int ab = 0 ; ab < commands.size(); ++ab ) {
		const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(commands.at(ab));
		if(m && m->matches(inputs)) {
			// execute the corresponding method
			YZCursor to=(this->*(m->motionMethod()))(YZNewMotionArgs(view, count, inputs.right( m->keySeq().length()), 
					inputs.left(m->keySeq().length()), usercount ));
			return to;
		}
	}
#endif
	return *view->getBufferCursor();
}


// MOTIONS

YZCursor YZCommandPool::moveLeft(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveLeft(&viewCursor, args.count, false, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveRight(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveRight(&viewCursor, args.count, false, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveLeftWrap( const YZNewMotionArgs & args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveLeft(&viewCursor, args.count, true, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveRightWrap( const YZNewMotionArgs & args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveRight(&viewCursor, args.count, true, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveDown(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	if ( args.standalone )
		args.view->moveDown(&viewCursor, args.count, true );
	else {//LINEWISE
		//update starting point
		args.view->gotoxy( 0, viewCursor.bufferY(), false );
		// end point
		args.view->moveDown( &viewCursor, args.count + 1, false );
		args.view->moveToStartOfLine( &viewCursor, true );
	}
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveUp(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	if ( args.standalone )
		args.view->moveUp(&viewCursor, args.count, true );
	else {//LINEWISE
		//update starting point
		if ( viewCursor.bufferY() == args.view->myBuffer()->lineCount() - 1 )
			args.view->moveToEndOfLine( &viewCursor, false );
		else
			args.view->gotoxy( 0, viewCursor.bufferY() + 1, false );
		// end point
		args.view->moveUp( &viewCursor, args.count, false );
		args.view->gotoxy ( &viewCursor, 0, viewCursor.bufferY(), true );
	}
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::movePageUp(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveUp(&viewCursor, args.view->getLinesVisible(), args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::movePageDown(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveDown(&viewCursor, args.view->getLinesVisible(), args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::matchPair(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	bool found = false;
	YZCursor pos = args.view->myBuffer()->action()->match( args.view, *viewCursor.buffer(), &found );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::findNext(const YZNewMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->forward( args.arg, found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZCommandPool::findBeforeNext(const YZNewMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->forwardBefore( args.arg, found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZCommandPool::findPrevious(const YZNewMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->reverse( args.arg, found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZCommandPool::findAfterPrevious(const YZNewMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->reverseAfter( args.arg, found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZCommandPool::repeatFind(const YZNewMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->searchAgain( found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZCommandPool::gotoSOL(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveToStartOfLine(&viewCursor,args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::gotoEOL(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveToEndOfLine(&viewCursor,args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveWordForward(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	YZCursor result( viewCursor.buffer() );
	unsigned int c = 0;
	QRegExp rex1("^\\w+\\s*");//a word with boundaries
	QRegExp rex2("^[^\\w\\s]+\\s*");//non-word chars with boundaries
	QRegExp ws("^\\s+");//whitespace

	while ( c < args.count ) { //for each word
		const QString& current = args.view->myBuffer()->textline( result.getY() );
//		if ( current.isNull() ) return false; //be safe ?

#if QT_VERSION < 0x040000
		int idx = rex1.search( current, result.getX(), QRegExp::CaretAtOffset );
#else
		int idx = rex1.indexIn( current, result.getX(), QRegExp::CaretAtOffset );
#endif
		int len = rex1.matchedLength();
		if ( idx == -1 ) {
#if QT_VERSION < 0x040000
			idx = rex2.search( current, result.getX(), QRegExp::CaretAtOffset );
#else
			idx = rex2.indexIn( current, result.getX(), QRegExp::CaretAtOffset );
#endif
			len = rex2.matchedLength();
		}
		if ( idx == -1 ) {
#if QT_VERSION < 0x040000
			idx = ws.search( current, result.getX(), QRegExp::CaretAtOffset );
#else
			idx = ws.indexIn( current, result.getX(), QRegExp::CaretAtOffset );
#endif
			len = ws.matchedLength();
		}
		if ( idx != -1 ) {
			yzDebug() << "Match at " << idx << " Matched length " << len << endl;
			c++; //one match
			result.setX( idx + len );
			if ( ( c < args.count || args.standalone ) && result.getX() == current.length() && result.getY() < args.view->myBuffer()->lineCount() - 1) {
				result.setY(result.getY() + 1);
#if QT_VERSION < 0x040000
				ws.search(args.view->myBuffer()->textline( result.getY() ));
				result.setX( QMAX( ws.matchedLength(), 0 ));
#else
				ws.indexIn(args.view->myBuffer()->textline( result.getY() ));
				result.setX( qMax( ws.matchedLength(), 0 ));
#endif
			}
		} else {
			if ( result.getY() >= args.view->myBuffer()->lineCount() - 1 ) {
				result.setX( current.length() );
				break;
			}
			result.setX(0);
			result.setY( result.getY() + 1 );
		}

	}
	if ( args.standalone ) 
		args.view->gotoxyAndStick( &result );

	return result;
}

QString invertQString( const QString& from ) {
	QString res = "";
	for ( int i = from.length() - 1 ; i >= 0; i-- )
		res.append( from[ i ] );
	return res;
}

YZCursor YZCommandPool::moveWordBackward(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	YZCursor result( viewCursor.buffer() );
	unsigned int c = 0;
	QRegExp rex1("^(\\w+)\\s*");//a word with boundaries
	QRegExp rex2("^([^\\w\\s]+)\\s*");//non-word chars with boundaries
	QRegExp rex3("^\\s+([^\\w\\s$]+|\\w+)");//whitespace

	while ( c < args.count ) { //for each word
		const QString& current = invertQString( args.view->myBuffer()->textline( result.getY() ) );
		int lineLength = current.length();
		int offset = lineLength - result.getX();
		yzDebug() << current << " at " << offset << endl;


#if QT_VERSION < 0x040000
		int idx = rex1.search( current, offset , QRegExp::CaretAtOffset );
#else
		int idx = rex1.indexIn( current, offset , QRegExp::CaretAtOffset );
#endif
		int len = rex1.cap( 1 ).length();
		yzDebug() << "rex1 : " << idx << "," << len << endl;
		if ( idx == -1 ) {
#if QT_VERSION < 0x040000
			idx = rex2.search( current, offset, QRegExp::CaretAtOffset );
#else
			idx = rex2.indexIn( current, offset, QRegExp::CaretAtOffset );
#endif
			len = rex2.cap( 1 ).length();
			yzDebug() << "rex2 : " << idx << "," << len << endl;
			if ( idx == -1 ) {
#if QT_VERSION < 0x040000
				idx = rex3.search( current, offset, QRegExp::CaretAtOffset );
#else
				idx = rex3.indexIn( current, offset, QRegExp::CaretAtOffset );
#endif
				len = rex3.matchedLength();
				yzDebug() << "rex3 : " << idx << "," << len << endl;
			}
		}
		if ( idx != -1 ) {
			yzDebug() << "Match at " << idx << " = " << lineLength - idx << " Matched length " << len << endl;
			c++; //one match
			result.setX( lineLength - idx - len );
		} else {
			if ( result.getY() == 0 ) break; //stop here
			yzDebug() << "Previous line " << result.getY() - 1 << endl;
			const QString& ncurrent = args.view->myBuffer()->textline( result.getY() - 1 );
			result.setX( ncurrent.length() );
			result.setY( result.getY() - 1 );
		}

	}

	if ( args.standalone ) 
		args.view->gotoxyAndStick( &result );

	return result;
}

YZCursor YZCommandPool::firstNonBlank(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveToFirstNonBlankOfLine(&viewCursor,args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::gotoMark( const YZNewMotionArgs &args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	bool found = false;
	YZCursorPos pos = args.view->myBuffer()->viewMarks()->get( args.arg, &found );
	if ( found )
		return *pos.bPos;
	else {
		yzDebug() << "WARNING! mark " << args.arg << " not found" << endl;
		return *viewCursor.buffer();
	}
}

YZCursor YZCommandPool::firstNonBlankNextLine( const YZNewMotionArgs &args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveDown(&viewCursor, args.count, args.standalone );
	args.view->moveToFirstNonBlankOfLine( &viewCursor, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::gotoLine(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	unsigned int line = 0;
	yzDebug() << "gotoLine " << args.cmd << "," << args.count << endl;
	if ( args.count > 0 ) line	= args.count - 1;

	if ( args.cmd == "gg"  || ( args.cmd == "G" && args.usercount ) )
		args.view->gotoLine( &viewCursor, line, args.standalone );
	else {
		if ( args.cmd == "G" )
			args.view->gotoLastLine( &viewCursor, args.standalone );
		else
			args.view->gotoLine( &viewCursor, 0, args.standalone );
	}
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::searchWord(const YZNewMotionArgs &args) {
	YZCursor from = *args.view->getBufferCursor();

	QString word = args.view->myBuffer()->getWordAt( from );
	if ( ! word.isNull() ) {
		yzDebug() << "searchWord : " << word << endl;
		YZCursor pos( args.view );
		bool found = true;
		bool moved = true;
		word = QRegExp::escape( word );
		if ( ! args.cmd.contains( 'g' ) ) {
			if ( word[ 0 ].isLetterOrNumber() || word[ 0 ] == '_' ) // \w
				word = "\\b" + word + "\\b";
			else
				word = word + "(?=[\\s\\w]|$)";
//				word = "(?=^|[\\s\\w])" + word + "(?=[\\s\\w]|$)"; seems that positive lookahead cannot work together...
		}
		for ( unsigned int i = 0; found && i < args.count; i++ ) {
			if ( args.cmd.contains('*') ) {
				pos = YZSession::me->search()->forward( args.view, word, &found, &from );
			} else {
				pos = YZSession::me->search()->backward( args.view, word, &found, &from );
			}
			if ( found ) {
				from.setCursor( pos );
				moved = true;
			}
		}
		if ( args.standalone && moved ) args.view->gotoxyAndStick( &from );
	}
	return from;
}

YZCursor YZCommandPool::searchNext(const YZNewMotionArgs &args) {
	YZCursor from = *args.view->getBufferCursor();
	YZCursor pos( args.view );
	bool found = true;
	bool moved = true;
	for ( unsigned int i = 0; found && i < args.count; i++ ) {
		pos = YZSession::me->search()->replayForward( args.view, &found, &from );
		if ( found ) {
			from.setCursor( pos );
			moved = true;
		}
	}
	if ( args.standalone && moved ) args.view->gotoxyAndStick( &from );
	return from;
}

YZCursor YZCommandPool::searchPrev(const YZNewMotionArgs &args) {
	YZCursor from = *args.view->getBufferCursor();
	YZCursor pos( args.view );
	bool found = true;
	bool moved = false;
	for ( unsigned int i = 0; found && i < args.count; i++ ) {
		pos = YZSession::me->search()->replayBackward( args.view, &found, &from );
		if ( found ) {
			from.setCursor( pos );
			moved = true;
		}
	}
	if ( args.standalone && moved ) args.view->gotoxyAndStick( &from );
	return from;
}

// COMMANDS

QString YZCommandPool::execMotion( const YZCommandArgs &args ) {
	const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(args.cmd);
	assert(m);
	YZCursor to = (this->*(m->motionMethod()))(YZNewMotionArgs(args.view, args.count, args.arg, args.cmd->keySeq(), args.usercount, true));
	args.view->gotoxy(to.getX(), to.getY());
	return QString::null;
}

QString YZCommandPool::appendAtEOL(const YZCommandArgs &args) {
	args.view->moveToEndOfLine();
	args.view->append();
	return QString::null;
}

QString YZCommandPool::append(const YZCommandArgs &args) {
	args.view->append();
	return QString::null;
}

QString YZCommandPool::change(const YZCommandArgs &args) {
	YZCursor from, to, temp;
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) {
		from = ( args.selection )[ 0 ].from();
		to = ( args.selection )[ 0 ].to();
	} else {
		from = *args.view->getBufferCursor();
		to = move(args.view, args.arg, args.count, args.usercount );
	}
	if(from > to) {
		temp = from;
		from = to;
		to = temp;
	}

	bool visualMode = args.view->getCurrentMode() >= YZView::YZ_VIEW_MODE_VISUAL;

	// if we delete to the end of the line, the cursor will be onto (before) the last
	// character, so we'll have to append. Otherwise, inserting is OK
#if QT_VERSION < 0x040000
	unsigned int eol_x = (visualMode ? (QMAX(0, int(args.view->myBuffer()->textline(to.getY()).length()) - 1))
			: args.view->myBuffer()->textline(to.getY()).length());
#else
	unsigned int eol_x = (visualMode ? (qMax(0, int(args.view->myBuffer()->textline(to.getY()).length()) - 1))
			: args.view->myBuffer()->textline(to.getY()).length());
#endif
	bool append = to.getX() >= eol_x;

	args.view->myBuffer()->action()->deleteArea(args.view, from, to, args.regs);
	args.view->commitNextUndo();

	if ( visualMode ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	}

	if(append)
		args.view->append();
	else
		args.view->gotoInsertMode();

	return QString::null;
}

QString YZCommandPool::changeLine(const YZCommandArgs &args) {
	args.view->myBuffer()->action()->deleteLine(args.view, *args.view->getBufferCursor(), args.count, args.regs);
	args.view->myBuffer()->action()->insertNewLine( args.view, 0, args.view->getBufferCursor()->getY() );
//	args.view->gotoInsertMode();
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::changeToEOL(const YZCommandArgs &args) {
	YZCursor to=move(args.view, "$", 1, false);
	args.view->myBuffer()->action()->deleteArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	args.view->append();
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::deleteLine(const YZCommandArgs &args) {
	args.view->myBuffer()->action()->deleteLine(args.view, *args.view->getBufferCursor(), args.count, args.regs);
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::deleteToEOL(const YZCommandArgs &args) {
	//in vim : 2d$ does not behave as d$d$, this is illogical ..., you cannot delete twice to end of line ...
	YZCursor to=move(args.view, "$", 1, false);
	args.view->myBuffer()->action()->deleteArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::gotoExMode(const YZCommandArgs &args) {
	args.view->gotoExMode();
	return QString::null;
}


QString YZCommandPool::insertAtSOL(const YZCommandArgs &args) {
	args.view->moveToFirstNonBlankOfLine();
	args.view->gotoInsertMode();
	return QString::null;
}

QString YZCommandPool::gotoInsertMode(const YZCommandArgs &args) {
	if ( args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL || args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL_LINE ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	} else if ( args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_REPLACE )
		args.view->leaveReplaceMode();
	args.view->gotoInsertMode();
	return QString::null;
}

QString YZCommandPool::gotoCommandMode(const YZCommandArgs &args) {
	if ( args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_INSERT )
		args.view->leaveInsertMode();
	else if ( args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_REPLACE )
		args.view->leaveReplaceMode();
	else if ( args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL || args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL_LINE  ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	}
	args.view->gotoCommandMode();
	return QString::null;
}

QString YZCommandPool::gotoLineAtTop(const YZCommandArgs &args) {
	unsigned int line;

	line = ( args.usercount ) ? args.count - 1 : args.view->drawLineNumber() - 1;
	args.view->alignViewVertically( line );
	args.view->gotoLine( line );
	args.view->moveToFirstNonBlankOfLine();
	return QString::null;
}

QString YZCommandPool::gotoLineAtCenter(const YZCommandArgs &args) {
	unsigned int line;

	line = ( args.usercount ) ? args.count - 1 : args.view->drawLineNumber() - 1;
	args.view->centerViewVertically( line );
	args.view->gotoLine( line );
	args.view->moveToFirstNonBlankOfLine();
	return QString::null;
}

QString YZCommandPool::gotoLineAtBottom(const YZCommandArgs &args) {
	unsigned int line;
	//unsigned int linesFromCenter;

	line = ( args.usercount ) ? args.count - 1 : args.view->drawLineNumber() - 1;
	//if ( line > linesFromCenter ) {
		args.view->bottomViewVertically( line );
	//}
	args.view->gotoLine( line );
	args.view->moveToFirstNonBlankOfLine();
	return QString::null;
}

QString YZCommandPool::gotoReplaceMode(const YZCommandArgs &args) {
	if ( args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL || args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL_LINE ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	} else if ( args.view->getCurrentMode()==YZView::YZ_VIEW_MODE_REPLACE )
		args.view->leaveReplaceMode();
	args.view->gotoReplaceMode();
	return QString::null;
}

QString YZCommandPool::gotoVisualLineMode(const YZCommandArgs &args) {
	if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	} else {
		args.view->gotoVisualMode( true );
	}
	return QString::null;
}

QString YZCommandPool::gotoVisualMode(const YZCommandArgs &args) {
	if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	} else {
		args.view->gotoVisualMode( false );
	}
	return QString::null;
}

QString YZCommandPool::insertLineAfter(const YZCommandArgs &args) {
	unsigned int y = args.view->getBufferCursor()->getY();
	args.view->myBuffer()->action()->insertNewLine( args.view, args.view->myBuffer()->textline( y ).length(), y );
	for ( unsigned int i = 1 ; i < args.count ; i++ )
		args.view->myBuffer()->action()->insertNewLine( args.view, 0, y + i );
	args.view->gotoInsertMode();
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::insertLineBefore(const YZCommandArgs &args) {
	unsigned int y = args.view->getBufferCursor()->getY();
	for ( unsigned int i = 0 ; i < args.count ; i++ )
		args.view->myBuffer()->action()->insertNewLine( args.view, 0, y );
	args.view->moveUp();
	args.view->gotoInsertMode();
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::joinLine(const YZCommandArgs &args) {
	for ( unsigned int i = 0; i < args.count; i++ ) 
		args.view->myBuffer()->action()->mergeNextLine( args.view, args.view->getBufferCursor()->getY(), true );
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::pasteAfter(const YZCommandArgs &args) {
	args.view->paste( args.regs[ 0 ], true );
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::pasteBefore(const YZCommandArgs &args) {
	args.view->paste( args.regs[ 0 ], false );
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::yankLine(const YZCommandArgs &args) {
	args.view->myBuffer()->action()->copyLine( args.view, *args.view->getBufferCursor(), args.count, args.regs );
	return QString::null;
}

QString YZCommandPool::yankToEOL(const YZCommandArgs &args) {
	YZCursor to=move(args.view, "$", 1, false);
	args.view->myBuffer()->action()->copyArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	return QString::null;
}

QString YZCommandPool::closeWithoutSaving(const YZCommandArgs &/*args*/) {
	YZSession::me->exitRequest( 0 );
	return QString::null;
}

QString YZCommandPool::saveAndClose(const YZCommandArgs &/*args*/) {
	YZSession::me->saveBufferExit();
	return QString::null;
}

QString YZCommandPool::searchBackwards(const YZCommandArgs &args) {
	args.view->gotoSearchMode(true);
	return QString::null;
}

QString YZCommandPool::searchForwards(const YZCommandArgs &args) {
	args.view->gotoSearchMode();
	return QString::null;
}

QString YZCommandPool::del(const YZCommandArgs &args) {
	bool entireLines = ( args.arg.length() > 0 && args.arg[ 0 ] == QChar('\'') ) || args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE;
	if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL )
		args.view->myBuffer()->action()->deleteArea(args.view, ( args.selection )[ 0 ].from(), ( args.selection )[ 0 ].to() , args.regs);
	else if ( entireLines ) {
		YZCursor from, to;
		if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE ) {
			from = ( args.selection )[ 0 ].from();
			to = ( args.selection )[ 0 ].to();
		} else {
			from = *args.view->getBufferCursor();
			to = move(args.view, args.arg, args.count, args.usercount);
			if ( from > to ) {
				YZCursor tmp( to );
				to.setCursor( from );
				from.setCursor( tmp );
			}
		}
		args.view->myBuffer()->action()->deleteLine( args.view, from.getY(), to.getY() - from.getY() + 1, args.regs );
	} else {
		YZCursor to=move(args.view, args.arg, args.count, args.usercount);
		args.view->myBuffer()->action()->deleteArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	}
	args.view->commitNextUndo();
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	}
	return QString::null;
}

QString YZCommandPool::yank(const YZCommandArgs &args) {
	bool entireLines =  ( args.arg.length() > 0 && args.arg[ 0 ] == QChar('\'') ) || args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE;
	if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL )
		args.view->myBuffer()->action()->copyArea(args.view, ( args.selection )[ 0 ].from(), ( args.selection )[ 0 ].to() , args.regs);
	else if ( entireLines ) {
		YZCursor from, to;
		if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE ) {
			from = ( args.selection )[ 0 ].from();
			to = ( args.selection )[ 0 ].to();
		} else {
			from = *args.view->getBufferCursor();
			to = move(args.view, args.arg, args.count, args.usercount);
			if ( from > to ) {
				YZCursor tmp( to );
				to.setCursor( from );
				from.setCursor( tmp );
			}
		}
		args.view->myBuffer()->action()->copyLine( args.view, from, to.getY() - from.getY() + 1, args.regs );
	} else {
		YZCursor to=move(args.view, args.arg, args.count, args.usercount);
		args.view->myBuffer()->action()->copyArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	}
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	}
	return QString::null;
}

QString YZCommandPool::mark(const YZCommandArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->myBuffer()->viewMarks()->add( args.arg, *viewCursor.buffer(), *viewCursor.screen() );
	return QString::null;
}

QString YZCommandPool::undo(const YZCommandArgs &args) {
	args.view->undo( args.count );
	return QString::null;
}

QString YZCommandPool::redo(const YZCommandArgs &args) {
	args.view->redo( args.count );
	return QString::null;
}

QString YZCommandPool::changeCase( const YZCommandArgs &args ) {
	YZCursor pos = *args.view->getBufferCursor();
	const QString line = args.view->myBuffer()->textline( pos.getY() );
	if ( ! line.isNull() ) {
		unsigned int length = line.length();
		unsigned int end = pos.getX() + args.count;
		for ( ; pos.getX() < length && pos.getX() < end; pos.setX( pos.getX() + 1 ) ) {
#if QT_VERSION < 0x040000
			QString ch = line.at( pos.getX() );
			if ( ch != ch.lower() )
				ch = ch.lower();
			else
				ch = ch.upper();
#else
			QString ch = QString(line.at( pos.getX() ));
			if ( ch != ch.toLower() )
				ch = ch.toLower();
			else
				ch = ch.toUpper();
#endif
			args.view->myBuffer()->action()->replaceChar( args.view, pos, ch );
		}
		args.view->commitNextUndo();
	}
	return QString::null;
}

QString YZCommandPool::macro( const YZCommandArgs &args ) {
	if ( args.view->isRecording() )
		args.view->stopRecordMacro();
	else
		args.view->recordMacro( args.regs );
	args.view->modeChanged();
	return QString::null;
}

QString YZCommandPool::replayMacro( const YZCommandArgs &args ) {
	args.view->purgeInputBuffer();
	if ( args.view->isRecording()) {
		yzDebug() << "User asked to play a macro he is currently recording, forget it !" << endl;
		if ( args.view->registersRecorded() == args.regs )
			return QString::null;
	}
#if QT_VERSION < 0x040000
	QValueList<QChar>::const_iterator it = args.regs.begin(), end = args.regs.end();
	for ( ; it != end; ++it )
		args.view->sendMultipleKey(YZSession::mRegisters.getRegister(*it)[ 0 ]);
#else
	for ( int ab = 0 ; ab < args.regs.size(); ++ab)
		args.view->sendMultipleKey(YZSession::mRegisters.getRegister(args.regs.at(ab))[0]);
#endif
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::deleteChar( const YZCommandArgs &args ) {
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL )
		args.view->myBuffer()->action()->deleteArea(args.view, ( args.selection )[ 0 ].from(), ( args.selection )[ 0 ].to() , args.regs);
	else
		args.view->myBuffer()->action()->deleteChar( args.view, args.view->getBufferCursor(), args.count );
	args.view->commitNextUndo();
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	}
	return QString::null;
}

QString YZCommandPool::redisplay( const YZCommandArgs &args ) {
	args.view->recalcScreen();
	return QString::null;
}

QString YZCommandPool::replace( const YZCommandArgs &args ) {
	YZCursor pos = args.view->getBufferCursor();
	args.view->myBuffer()->action()->replaceChar( args.view, pos, args.arg );
	args.view->gotoxy(pos.getX(),pos.getY(),true);
	args.view->updateStickyCol();
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::abort( const YZCommandArgs& args) {
	//actually it's a no-op here, inputBuffer will be purged after
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) {
		args.view->leaveVisualMode();
		args.view->gotoPreviousMode();
	}
	return QString::null;
}

QString YZCommandPool::delkey( const YZCommandArgs &args ) {
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) {
#if QT_VERSION < 0x040000
			args.view->myBuffer()->action()->deleteArea( args.view, (args.selection)[0].from(), (args.selection)[0].to(), ( QValueList<QChar>() << QChar( '\"' ) ));
#else
			args.view->myBuffer()->action()->deleteArea( args.view, (args.selection)[0].from(), (args.selection)[0].to(), ( QList<QChar>() << QChar( '\"' ) ));
#endif
			args.view->leaveVisualMode();
			args.view->gotoPreviousMode();
	} else
			args.view->myBuffer()->action()->deleteChar( args.view, *(args.view->getBufferCursor()), 1);
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::indent( const YZCommandArgs& args ) {
	unsigned int fromY, toY;
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) {
		fromY = (args.selection)[0].from().getY();
		toY = (args.selection)[0].to().getY();
	} else {
		fromY = args.view->getBufferCursor()->getY();
		toY = move(args.view, args.arg, args.count, args.usercount).getY();
		if ( fromY > toY ) {
			fromY = toY;
			toY = args.view->getBufferCursor()->getY();
		}
	}
	unsigned int maxY = args.view->myBuffer()->lineCount() - 1;
	if ( toY > maxY ) toY = maxY;
	int factor = ( args.cmd->keySeq()[0] == '<' ? -1 : 1 ) * args.count;
	for ( unsigned int l = fromY; l <= toY; l++ ) {
		args.view->myBuffer()->action()->indentLine( args.view, l, factor );
	}
	args.view->commitNextUndo();
	return QString::null;
}

