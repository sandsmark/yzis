/*
Copyright (c) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
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

#ifndef NY_SESSION_H
#define NY_SESSION_H

/**
 * NYSession - Ncurses main class for the whole session
 */

/* yzis */
#include "nyview.h"
#include "session.h"

/**
  * @short Implementation of YSession for the NCurses frontend.
  */
class NYSession : public QObject, public YSession
{
    Q_OBJECT

public:
    /**
     *  Creates one and the only session instance.
     *  Should be called from main() before any other yzis object
     *  construction.
     */
    static void createInstance();

    /*
     * YSession interface :
     */
    virtual bool guiQuit(int errorCode) override;
    virtual void guiPopupMessage(const QString &message) override;
    virtual bool guiPromptYesNo(const QString &title, const QString &message) override;
    virtual int guiPromptYesNoCancel(const QString &title, const QString &message) override;
    virtual void guiSplitHorizontally(YView *view) override;
    virtual void guiSetClipboardText(const QString &text, Clipboard::Mode mode) override;

    virtual void guiSetFocusCommandLine();
    virtual void guiSetFocusMainWindow();

protected:
    virtual YView *guiCreateView(YBuffer *buffer) override;
    virtual void guiDeleteView(YView *view) override;

private:
    NYSession();
    NYSession(const NYSession &); // disable copy
    NYSession &operator=(const NYSession &); // disable copy
    virtual ~NYSession();

    /**
     * Fill the map of keycodes ncurses->Qt
     */
    void initialiseKeycodes();

    virtual void guiChangeCurrentView(YView *) override;

    /**
      * mapping ncurses->qt for keycodes
      */
    static QMap<int, YKey> keycodes; // map Ncurses to Qt codes
    QString m_initialCommand;

public Q_SLOTS:
    /** To be called by single shot timer, when the gui is ready
      * and the Qt event loop is running.
      */
    void frontendGuiReady();
    bool processInput(int);
};

#endif // NY_SESSION_H
