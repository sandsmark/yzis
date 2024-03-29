/*
Copyright (c) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
Copyright (c) 2004-2005 Mickael Marchand <marchand@kde.org>

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

/* Nyzis */
#include "nysession.h"

/* Yzis */
#include "buffer.h"
#include "debug.h"

/* Std */
#include <unistd.h>
#include <ctype.h>

#define dbg() yzDebug("NYSession")
#define warn() yzWarning("NYSession")
#define err() yzError("NYSession")

QMap<int, YKey> NYSession::keycodes; // map Ncurses to Qt codes

void NYSession::createInstance()
{
    dbg() << "createInstance()" << endl;
    // such allocation (i.e. not "new NYSession") will ensure that
    // "instance" object will be properly and automatically deleted
    // when program exits
    static NYSession instance;
    setInstance(&instance);
}

NYSession::NYSession() :
    YSession()
{
    dbg() << "NYSession()" << endl;
    /* init screen */
    (void)initscr(); /* initialize the curses library */
    keypad(stdscr, true); /* enable keyboard mapping */
    (void)nonl(); /* tell curses not to do NL->CR/NL on output */
    (void)cbreak(); /* take input chars one at a time, no wait for \n */
    (void)noecho(); /* echo input - in color */
    intrflush(stdscr, false);
    // notimeout( stdscr, true ); // XXX conflict with nodelay ?? if you want to test half or no delay, comment this line.
    halfdelay(1);
    // nodelay( stdscr, true );
    set_escdelay(100);
    wattron(stdscr, A_STANDOUT); // will be herited by subwin
    use_default_colors();

    if (has_colors()) {
        start_color();
    }

    initialiseKeycodes();
}

NYSession::~NYSession()
{
    dbg() << "~NYSession()" << endl;
}

void NYSession::frontendGuiReady()
{
    dbg() << "frontendGuiReady()" << endl;
    YSession::self()->frontendGuiReady();
}

bool NYSession::processInput(int /*fd*/)
{
    YASSERT_MSG(currentView(), "NYSession::event_loop : arghhhhhhh event_loop called with no currentView");
    wint_t c;
    // this one is not blocking thanx to the nodelay() call
    int ret = get_wch(&c);

    if (ret == ERR) {
        return false;
    }

    // int c = getch();

    // we know what this is..
    switch (c) {
    //  case ERR: // nothing to be done
    //   return false;
    case KEY_RESIZE: // do nothing with this one
        return true;

    // don't do  currentView->sendKey(QString(QChar(0x60+c)),"<CTRL>");
    // on all iscntrl(c), it would break <esc> and <enter>....
    case 0x01: // ^a
    case 0x02: // ^b
    case 0x05: // ^e
    case 0x06: // ^f
    case 0x07: // ^g
    case 0x08: // ^h

    //  case 0x09: // ^i  <----- tab
    //  case 0x0a: // ^j  <----- enter
    case 0x0b: // ^k
    case 0x0c: // ^l // important, tested

    //  case 0x0d: // ^m  <----- return
    case 0x0e: // ^n // important, tested
    case 0x0f: // ^o // important, tested
    case 0x10: // ^p
    case 0x11: // ^q
    case 0x12: // ^r // important, tested

    //  case 0x13: // ^s
    //  case 0x14: // ^t
    //  case 0x15: // ^u
    //  case 0x16: // ^v
    //  case 0x17: // ^w
    case 0x18: // ^x // important, tested
    case 0x19: // ^y
    case 0x1a: // ^z
        sendKey(currentView(), YKey(0x60 + c, Qt::ControlModifier));
        return true;
    } // switch

    if (c == 0x1d) {
        sendKey(currentView(), YKey(Qt::Key_BracketRight, Qt::ControlModifier));
    }

    if (keycodes.contains(c)) {
        sendKey(currentView(), keycodes[c]);
        return true;
    }

    // remaining cases
    if (c & KEY_CODE_YES) { // ncurses special key
        err() << "*************** Unhandled"
              << "ncurses key code, please report : " << (int)c << endl;
        return true;
    }

    Qt::KeyboardModifiers modifiers = Qt::NoModifier;

    /* if ( c & 0200 ) {
      // heuristic, alt-x is x|0200..
      modifiers += "<ALT>";
      c &= ~0200;
     } */
    if (c >= KEY_MAX) { // non-ascii key
        err() << "*************** Unhandled"
              << "and very strange (>KEY_MAX) char received from ncurses, please report : " << (int)c << endl;
        return true;
    }

    // detect/remove the ncurses 'control' flag
    if (iscntrl(c)) {
        modifiers |= Qt::ControlModifier;
        c += 96;
    }

    //yzDebug() << "sendKey < " << c << " (" << QString( QChar( c ) ) << ") modifiers=" << modifiers << endl;
    //TODO: META
    sendKey(currentView(), YKey(c, modifiers));
    //    sendKey( currentView(), YKey( c, modifiers, QChar(c)));
    return true;
}

void NYSession::guiSetClipboardText(const QString &, Clipboard::Mode)
{
    // XXX
}

bool NYSession::guiQuit(int errorCode)
{
    dbg() << "guiQuit(" << errorCode << ")" << endl;
    exit(errorCode);
    return true;
}

void NYSession::guiSetFocusMainWindow()
{
    dbg() << "guiSetFocusMainWindow()" << endl;
    NYView *yv = static_cast<NYView *>(currentView());
    yv->guiSetFocusMainWindow();
}

void NYSession::guiSetFocusCommandLine()
{
    dbg() << "guiSetFocusCommandLine()" << endl;
    NYView *yv = static_cast<NYView *>(currentView());
    yv->guiSetFocusCommandLine();
}

void NYSession::guiChangeCurrentView(YView *view)
{
    dbg() << "changeCurrentView( " << view->toString() << ")" << endl;
    NYView *cur = static_cast<NYView *>(currentView());
    NYView *v = static_cast<NYView *>(view);
    YASSERT(view);

    if (cur == v) {
        warn() << "changeCurrentView() called with same view.." << endl;
        return;
    }

    if (cur) {
        cur->unmap();
    }

    v->map();
    v->sendRefreshEvent();
}

YView *NYSession::guiCreateView(YBuffer *buffer)
{
    dbg() << "doCreateView( " << buffer->toString() << ")" << endl;
    YASSERT(buffer);
    NYView *v = new NYView(buffer);
    YASSERT_MSG(v, "NYSession::createView : failed creating a new NYView");
    return v;
}

void NYSession::guiPopupMessage(const QString &_message)
{
    int nl, nc;
    QString anyKeyMsg = tr("(Press any key)");
    int length = anyKeyMsg.length();
    QString message = _message.simplified();
#if 0
    int y;
    message.simplifyWhiteSpace();
    nc = message.length();
    nl = 1;

    if(nc > COLS - 4 && COLS - 4 > 0) {
        nl = nc / COLS - 4;
        nc = COLS - 4;
    }

    WINDOW *popup = newwin(nl + 4, nc + 4, (LINES - nl) / 2, (COLS - nc) / 2);
    box(popup, 0, 0);
    y = 2;

    while(nl) {
        mvwaddstr(popup, y, 2, message.mid(y * nc, nc).latin1());
        y++;
        nl--;
    }

    // TODO : use QString QString::section
#else
    nc = message.length();
    nl = 6;
    WINDOW *popup = newwin(nl, nc + 4, (LINES - nl) / 2, (COLS - nc) / 2);
    box(popup, 0, 0);
    mvwaddstr(popup, 2, 2, message.toLocal8Bit().constData());
    mvwaddstr(popup, 4, ((nc + 4) - length) / 2, anyKeyMsg.toLocal8Bit().constData()); // Center the text.
    // TODO : use QString QString::section
#endif
    wrefresh(popup);
    refresh();
    nocbreak(); // Leave half-delay mode for a moment.
    cbreak(); // Enter cbreak-mode.
    wgetch(popup);
    halfdelay(1); // Go back to the halfdelay-mode.
    delwin(popup);

    if (currentView()) { // view is not up yet, let's output that to stderr maybe ?
        currentView()->sendRefreshEvent();
    } else {
        fputs(qp(message), stderr);
    }
}

void NYSession::guiDeleteView(YView *view)
{
    dbg() << "guiDeleteView(" << view->toString() << ")" << endl;
    NYView *oldview = dynamic_cast<NYView *>(view);
    Q_ASSERT(oldview);
    NYView *newview = dynamic_cast<NYView *>(currentView());
    Q_ASSERT(newview);
    newview->guiSetCommandLineText("");
    newview->sendRefreshEvent();
    dbg() << "guiDeleteView(): delete oldview;" << endl;
    delete oldview;
    // dumpObjectTree();
    dbg() << "guiDeleteView(): done" << endl;
}

bool NYSession::guiPromptYesNo(const QString & /*title*/, const QString & /*message*/)
{
    //TODO
    return true;
}

int NYSession::guiPromptYesNoCancel(const QString & /*title*/, const QString & /*message*/)
{
    //TODO
    return 0; //return yes for now...
}

void NYSession::guiSplitHorizontally(YView * /*view*/)
{
    //TODO
}

void NYSession::initialiseKeycodes()
{
    keycodes.clear();
    // ascii stuff
    keycodes[9] = YKey(Qt::Key_Tab);
    keycodes[10] = YKey(Qt::Key_Enter); // enter
    keycodes[13] = YKey(Qt::Key_Return); // return
    keycodes[27] = YKey(Qt::Key_Escape);
    keycodes[127] = YKey(Qt::Key_Backspace);
    //keycodes[ KEY_CODE_YES ] = ;
    //keycodes[ KEY_MIN ] = ;
    //    keycodes[ KEY_BREAK ] = YKey(Qt::Key_Break);
    //keycodes[ KEY_SRESET ] = ;
    //keycodes[ KEY_RESET ] = ;
    keycodes[KEY_DOWN] = YKey(Qt::Key_Down);
    keycodes[KEY_UP] = YKey(Qt::Key_Up);
    keycodes[KEY_LEFT] = YKey(Qt::Key_Left);
    keycodes[KEY_RIGHT] = YKey(Qt::Key_Right);
    keycodes[KEY_HOME] = YKey(Qt::Key_Home);
    keycodes[KEY_BACKSPACE] = YKey(Qt::Key_Backspace);
    //keycodes[ KEY_F0 ] = Qt::Key_F0;
    keycodes[KEY_F(1)] = YKey(Qt::Key_F1);
    keycodes[KEY_F(2)] = YKey(Qt::Key_F2);
    keycodes[KEY_F(3)] = YKey(Qt::Key_F3);
    keycodes[KEY_F(4)] = YKey(Qt::Key_F4);
    keycodes[KEY_F(5)] = YKey(Qt::Key_F5);
    keycodes[KEY_F(6)] = YKey(Qt::Key_F6);
    keycodes[KEY_F(7)] = YKey(Qt::Key_F7);
    keycodes[KEY_F(8)] = YKey(Qt::Key_F8);
    keycodes[KEY_F(9)] = YKey(Qt::Key_F9);
    keycodes[KEY_F(10)] = YKey(Qt::Key_F10);
    keycodes[KEY_F(11)] = YKey(Qt::Key_F11);
    keycodes[KEY_F(12)] = YKey(Qt::Key_F12);
    //keycodes[ KEY_DL ] = ;
    //keycodes[ KEY_IL ] = ;
    keycodes[KEY_DC] = YKey(Qt::Key_Delete);
    keycodes[KEY_IC] = YKey(Qt::Key_Insert);
    keycodes[Qt::Key_Insert] = YKey(Qt::Key_Insert);
    //keycodes[ KEY_EIC ] = ;
    keycodes[KEY_CLEAR] = YKey(Qt::Key_Clear);
    //keycodes[ KEY_EOS ] = ;
    //keycodes[ KEY_EOL ] = ;
    //keycodes[ KEY_SF ] = ;
    //keycodes[ KEY_SR ] = ;
    keycodes[KEY_NPAGE] = YKey(Qt::Key_PageDown);
    keycodes[KEY_PPAGE] = YKey(Qt::Key_PageUp);
    //keycodes[ KEY_STAB ] = ;
    //keycodes[ KEY_CTAB ] = ;
    //keycodes[ KEY_CATAB ] = ;
    keycodes[KEY_ENTER] = YKey(Qt::Key_Enter);
    ;
    keycodes[KEY_PRINT] = YKey(Qt::Key_Print);
    //keycodes[ KEY_LL ] = ;
    keycodes[KEY_A1] = YKey(Qt::Key_Home);
    //    keycodes[ KEY_A3 ] = YKey(Qt::Key_Prior);
    //keycodes[ KEY_B2 ] = ;
    keycodes[KEY_C1] = YKey(Qt::Key_End);
    //    keycodes[ KEY_C3 ] = YKey(Qt::Key_Next);
    keycodes[KEY_BTAB] = YKey(Qt::Key_Backtab);
    //keycodes[ KEY_BEG ] = ;
    //keycodes[ KEY_CANCEL ] = ;
    //keycodes[ KEY_CLOSE ] = ;
    //keycodes[ KEY_COMMAND ] = ;
    //keycodes[ KEY_COPY ] = ;
    //keycodes[ KEY_CREATE ] = ;
    keycodes[KEY_END] = YKey(Qt::Key_End);
    //keycodes[ KEY_EXIT ] = ;
    //keycodes[ KEY_FIND ] = ;
    //keycodes[ KEY_HELP ] = ;
    //keycodes[ KEY_MARK ] = ;
    //keycodes[ KEY_MESSAGE ] = ;
    //keycodes[ KEY_MOVE ] = ;
    //keycodes[ KEY_NEXT ] = ;
    //keycodes[ KEY_OPEN ] = ;
    //keycodes[ KEY_OPTIONS ] = ;
    //keycodes[ KEY_PREVIOUS ] = ;
    //keycodes[ KEY_REDO ] = ;
    //keycodes[ KEY_REFERENCE ] = ;
    //keycodes[ KEY_REFRESH ] = ;
    //keycodes[ KEY_REPLACE ] = ;
    //keycodes[ KEY_RESTART ] = ;
    //keycodes[ KEY_RESUME ] = ;
    //keycodes[ KEY_SAVE ] = ;
    //keycodes[ KEY_SBEG ] = ;
    /*keycodes[ KEY_SCANCEL ] = ;
     keycodes[ KEY_SCOMMAND ] = ;
     keycodes[ KEY_SCOPY ] = ;
     keycodes[ KEY_SCREATE ] = ;
     keycodes[ KEY_SDC ] = ;
     keycodes[ KEY_SDL ] = ;
     keycodes[ KEY_SELECT ] = ;
     keycodes[ KEY_SEND ] = ;
     keycodes[ KEY_SEOL ] = ;
     keycodes[ KEY_SEXIT ] = ;
     keycodes[ KEY_SFIND ] = ;
     keycodes[ KEY_SHELP ] = ;
     keycodes[ KEY_SHOME ] = ;
     keycodes[ KEY_SIC ] = ;
     keycodes[ KEY_SLEFT ] = ;
     keycodes[ KEY_SMESSAGE ] = ;
     keycodes[ KEY_SMOVE ] = ;
     keycodes[ KEY_SNEXT ] = ;
     keycodes[ KEY_SOPTIONS ] = ;
     keycodes[ KEY_SPREVIOUS ] = ;
     keycodes[ KEY_SPRINT ] = ;
     keycodes[ KEY_SREDO ] = ;
     keycodes[ KEY_SREPLACE ] = ;
     keycodes[ KEY_SRIGHT ] = ;
     keycodes[ KEY_SRSUME ] = ;
     keycodes[ KEY_SSAVE ] = ;
     keycodes[ KEY_SSUSPEND ] = ;
     keycodes[ KEY_SUNDO ] = ;
     keycodes[ KEY_SUSPEND ] = ;
     keycodes[ KEY_UNDO ] = ;
     keycodes[ KEY_MOUSE ] = ;
     keycodes[ KEY_RESIZE ] = ;
     keycodes[ KEY_EVENT ] = ;
     keycodes[ KEY_MAX ] = ;*/
}
