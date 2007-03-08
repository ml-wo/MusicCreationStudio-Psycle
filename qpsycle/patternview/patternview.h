/***************************************************************************
*   Copyright (C) 2006 by  Neil Mather   *
*   nmather@sourceforge   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
 #ifndef PATTERNVIEW_H
 #define PATTERNVIEW_H

 #include <QGraphicsView>
 #include <QGraphicsScene>

 #include "linenumbercolumn.h"
#include "patterngrid.h"
 #include "psycore/song.h"
 #include "psycore/singlepattern.h"

 class QToolBar;
 class QComboBox;
 class QGridLayout;
 class QAction;

 class LineNumberColumn;
 class PatternGrid;
 class PatCursor;

 class PatternView : public QGraphicsView
 {
     Q_OBJECT

 public:
     PatternView( psy::core::Song *song );

     int rowHeight() const;
     int numberOfLines() const;
     int numberOfTracks() const;
     void setNumberOfTracks( int numTracks );
     int trackWidth() const;

     void setPattern( psy::core::SinglePattern *pattern );
     psy::core::SinglePattern *pattern();

     void enterNote( const PatCursor & cursor, int note );

    void setSelectedMachineIndex( int idx );
    int selectedMachineIndex() const;

    PatternGrid* patternGrid() { return patGrid_; }


 private:
    void createToolBar();

    psy::core::Song *song_;

    QGraphicsScene *scene_;

    LineNumberColumn *lineNumCol_;
    PatternGrid *patGrid_;
    psy::core::SinglePattern *pattern_;

    int numberOfTracks_;
    QGridLayout *layout;

    QToolBar *toolBar_;
    QComboBox *meterCbx_;
    QComboBox *patternCbx_;

    QAction *delBarAct_;

    int selectedMacIdx_;

 };

 #endif
