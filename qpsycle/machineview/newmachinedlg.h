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
#ifndef NEWMACHINEDLG_H
#define NEWMACHINEDLG_H

#include <map>

#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidgetItem>

#include "pluginfinder.h"

class QDialog;

class NewMachineDlg : public QDialog
{
     Q_OBJECT

 public:
     NewMachineDlg(QWidget *parent = 0);

    const psy::core::PluginFinderKey & pluginKey() const;

public slots:
    void currentItemChanged( QListWidgetItem *current, QListWidgetItem *previous );

 private:
     QDialogButtonBox *buttonBox;
    const psy::core::PluginFinder *finder_;


    psy::core::PluginFinderKey selectedKey_;
	std::map< QListWidgetItem* , psy::core::PluginFinderKey > pluginIdentify_;

    void setPlugin( QListWidgetItem* item );


};

 #endif
