/* This file is part of the Yzis libraries
*  Copyright (C) 2004 Loic Pauleve <panard@inzenet.org>
*  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>
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

/* QYzis */
#include "qycursor.h"
#include "qyedit.h"
#include "qyview.h"

/* Yzis */
#include "debug.h"

#define deepdbg() yzDeepDebug("QYCursor")
#define dbg() yzDebug("QYCursor")
#define err() yzError("QYCursor")

QYCursor::QYCursor(QYView *view, QYEdit *edit, CursorShape shape) :
    QWidget(edit)
{
    mView = view;
    mEdit = edit;
    setAutoFillBackground(false);
    setAttribute(Qt::WA_OpaquePaintEvent);
    move(0, 0);
    setCursorShape(shape);
}

QYCursor::~QYCursor()
{
}

QYCursor::CursorShape QYCursor::shape() const
{
    return mCursorShape;
}
void QYCursor::setCursorShape(CursorShape shape)
{
    if (shape == mCursorShape) {
        return;
    }

    mCursorShape = shape;
    int w = parentWidget()->fontMetrics().maxWidth();

    if (w == 0) {
        w = parentWidget()->fontMetrics().horizontalAdvance("W");
    }

    int h = parentWidget()->fontMetrics().lineSpacing();

    if (mCursorShape == CursorVbar) {
        w = 2;
    }

    resize(w, h);
}

void QYCursor::paintEvent(QPaintEvent *pe)
{
    Q_UNUSED(pe);
    YCursor my_pos = mView->getRowColumnCursor();
    YDrawBufferConstIterator it = mEdit->mView->mDrawBuffer.const_iterator(YInterval(my_pos, my_pos), yzis::ScreenInterval);
    const YDrawCell cell = it.drawCellInfo().cell;
    QColor cbg, cfg;
    deepdbg().SPrintf("paintEvent(): cell string='%s'", qp(cell.content()));

    if (cell.backgroundColor().isValid()) {
        deepdbg() << "paintEvent(): valid cell.backgroundColor()" << endl;
        cbg = QColor(cell.backgroundColor().rgb());
        //cbg = QColor( Qt::red );
    } else {
        deepdbg() << "paintEvent(): invalid cell.backgroundColor()" << endl;
        cbg = parentWidget()->palette().color(QPalette::Window);
        //cbg = QColor(Qt::magenta);
    }

    deepdbg() << "paintEvent(): cell background=" << cbg.name() << endl;

    if (cell.foregroundColor().isValid()) {
        deepdbg() << "paintEvent(): valid cell.foregroundColor()" << endl;
        cfg = QColor(cell.foregroundColor().rgb());
        //cfg = QColor(Qt::blue);
    } else {
        deepdbg() << "paintEvent(): invalid cell.foregroundColor()" << endl;
        cfg = parentWidget()->palette().color(QPalette::WindowText);
        //cfg = QColor(Qt::cyan);
    }

    deepdbg() << "paintEvent(): cell foreground=" << cfg.name() << endl;
    QPainter p(this);
    QRect r = rect();
    CursorShape s = shape();
    deepdbg() << "paintEvent(): shape=" << s << endl;

    switch (s) {
    case CursorFilledRect:
        p.setPen(cbg);
        p.setBackground(cfg);
        // erase with cell foreground
        p.eraseRect(rect());
        // paint character with cell background
        p.drawText(rect(), cell.content());
        break;

    case CursorFrameRect:
        p.setPen(cfg);
        p.setBackground(cbg);
        // erase with cell background
        p.eraseRect(r);
        // paint character with cell foreground
        p.drawText(r, cell.content());
        // paint rect with cell foreground
        r.adjust(0, 0, -1, -1);
        p.drawRect(r);
        break;

    case CursorVbar:
        r.setWidth(2);
        p.fillRect(r, QBrush(cfg));
        break;

    case CursorHbar:
        // erase with cell background
        p.eraseRect(r);
        // paint character with cell foreground
        p.drawText(r, cell.content());
        r.setTop(r.bottom() - 2);
        p.fillRect(r, QBrush(cfg));
        break;

    case CursorHidden:
        // erase with cell background
        p.eraseRect(r);
        // paint character with cell foreground
        p.drawText(r, cell.content());
        break;
    }
}

YDebugStream &operator<<(YDebugStream &out, const QYCursor::CursorShape &shape)
{
    switch (shape) {
    case QYCursor::CursorFilledRect:
        out << "CursorFilledRect";
        break;

    case QYCursor::CursorVbar:
        out << "CursorVbar";
        break;

    case QYCursor::CursorHbar:
        out << "CursorHbar";
        break;

    case QYCursor::CursorFrameRect:
        out << "CursorFrameRect";
        break;

    case QYCursor::CursorHidden:
        out << "CursorHidden";
        break;
    }

    return out;
}
