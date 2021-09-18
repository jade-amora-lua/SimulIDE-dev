/***************************************************************************
 *   Copyright (C) 2021 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "enumval.h"
#include "component.h"

EnumVal::EnumVal( QWidget* parent )
       : PropVal( parent )
{
    setupUi(this);
    m_propName = "";
    m_component = NULL;
    m_blocked = false;
}
EnumVal::~EnumVal() {}

void EnumVal::setPropName( QString name, QString caption )
{
    m_propName = name;
    valLabel->setText( caption );
}

void EnumVal::setup( Component* comp, QString )
{
    m_blocked = true;
    m_component = comp;

    QVariant   v = comp->property( m_propName.toUtf8() );
    const QMetaObject* meta = comp->metaObject();
    QMetaProperty prop = meta->property(meta->indexOfProperty(qPrintable(m_propName)));
    if( prop.isEnumType() )
    {
        QMetaEnum qenum = prop.enumerator();
        for( int i=0; i<qenum.keyCount(); ++i ) valueBox->addItem( qenum.key(i) );
        valueBox->setCurrentIndex( v.toInt() );
    }
    else
    {
        QStringList enums = comp->getEnums();
        enums.sort();
        for( QString val : enums ) valueBox->addItem( val );
        valueBox->setCurrentText( v.toString() );
    }
    this->adjustSize();
    m_blocked = false;
}

void EnumVal::on_valueBox_currentIndexChanged( int val )
{
    if( m_blocked ) return;
    m_component->setProperty( m_propName.toUtf8(), val );
}

void EnumVal::updtValues()
{

}

