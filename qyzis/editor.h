/* This file is part of QYzis
*  Copyright (C) 2006 Loic Pauleve <panard@inzenet.org>
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
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#ifndef QYZISEDIT_H
#define QYZISEDIT_H

/* Qt */
#include <qpainter.h>
#include <qevent.h>
#include <qmap.h>
#include <qfontmetrics.h>
#include <qfont.h>
#include <qnamespace.h>
#include <qsignalmapper.h>

/* Project */
#include "libyzis/drawbuffer.h"
#include "qyziscursor.h"
#include "action.h"

class QYZisView;
class QYZisCursor;

/**
 * QYZis Painter Widget
 */
class QYZisEdit : public QWidget
{
    Q_OBJECT


public :
    QYZisEdit(QYZisView *parent = 0);
    virtual ~QYZisEdit();

    //erase all text, and set new text
    void setText (const QString& );

    //append text
    void append ( const QString& );

    //move cursor to position column, line relative to viewport
    void setCursor(int c, int l);
    void scroll( int dx, int dy );

    /** Return a cursor shape according to the current mode and focus */
    QYZisCursor::CursorShape cursorShape();

    /** Adjust the cursor shape.
      *
      * Set the cursor shape returned bye cursorShape() to the current cursor.
      */
    void updateCursor();

    // update text area
    void updateArea( );

    void setPalette( const QPalette& p, qreal opacity );

    const QString& convertKey( int key );

    unsigned int spaceWidth;

    void registerModifierKeys( const QString& keys );
    void unregisterModifierKeys( const QString& keys );

    QPoint translatePositionToReal( const YZCursor& c ) const;
    YZCursor translateRealToPosition( const QPoint& p, bool ceil = false ) const;
    YZCursor translateRealToAbsolutePosition( const QPoint& p, bool ceil = false ) const;

    QVariant inputMethodQuery ( Qt::InputMethodQuery query ) const;

    QYZisView* view() const;

public slots :
    void sendMappedKey( const QString& keys );


protected:
    void guiPaintEvent( const YZSelection& drawMap );
    void guiDrawCell( QPoint pos, const YZDrawCell& cell, QPainter* p );
    void guiDrawClearToEOL( QPoint pos, const QChar& clearChar, QPainter* p );

    //intercept tabs
    virtual bool event(QEvent*);

    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);

    //normal keypressEvents processing
    void keyPressEvent (QKeyEvent *);

    //mouse events
    void mousePressEvent (QMouseEvent *);

    //mouse move event
    void mouseMoveEvent( QMouseEvent *);

    // mousebutton released
    //  void mouseReleaseEvent( QMouseEvent *);

    //insert text at line
    void insetTextAt(const QString&, int line);

    //insert a char at idx on line ....
    void insertCharAt(QChar, int);

    //replace a char at idx on line ....
    void replaceCharAt( QChar, int );

    virtual void focusInEvent( QFocusEvent * );
    virtual void focusOutEvent( QFocusEvent * );

    // for InputMethod
    void inputMethodEvent ( QInputMethodEvent * );

private :
    void initKeys();
    QSignalMapper* signalMapper;
    QString keysToShortcut( const QString& keys );

    /* area to use */
    QRect m_useArea;

    QYZisView *mParent;
    QYZisCursor* mCursor;

    bool isFontFixed;

    static QMap<int, QString> keys;

    friend class QYZisCursor;
    friend class QYZisView;
};

#endif
