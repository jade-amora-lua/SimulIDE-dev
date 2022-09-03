/***************************************************************************
 *   Copyright (C) 2022 by santiago González                               *
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

#include <QDebug>

#include <math.h>

#include "ioport.h"

IoPort::IoPort( QString name )
{
    m_name = name;
    m_shortName = "P"+ name.right(1);
    m_numPins = 0;
}
IoPort::~IoPort(){}

void IoPort::reset()
{
    m_pinState = 0;
    m_pinDirection = 0;
    for( IoPin* pin : m_pins ) pin->setOutState( 0 );
    for( IoPin* pin : m_pins ) pin->setPinMode( input );
}

void IoPort::setOutState( uint32_t val )
{
    /*uint8_t changed = m_pinState ^ val; // See which Pins have actually changed
    if( changed == 0 ) return;

    for( int i=0; i<m_numPins; ++i ){
        if( changed & (1<<i) ) m_pins[i]->setOutState( (val & (1<<i)) > 0 ); // Pin changed
    }*/
    for( int i=0; i<m_numPins; ++i )
    {
        bool state = (val & 1<<i) > 0;
        if( state != m_pins[i]->getOutState() )
            m_pins[i]->setOutState( state );
    }
}

uint32_t IoPort::getInpState()
{
    uint32_t data = 0;
    for( int i=0; i<m_numPins; ++i )
        if( m_pins[i]->getInpState() ) data += (1 << i);
    return data;
}

void IoPort::setDirection( uint32_t val )
{
    uint8_t changed = m_pinDirection ^ val;  // See which Pins have actually changed
    if( changed == 0 ) return;

    for( int i=0; i<m_numPins; ++i ){
        if( changed & 1<< i) m_pins[i]->setPinMode( ((val & (1<<i)) > 0) ? output : input ); // Pin changed
}   }

void IoPort::setPinMode( pinMode_t mode )
{
    for( IoPin* pin : m_pins ) pin->setPinMode( mode );
}

void IoPort::portVoltChanged( eElement* el, bool ch )
{
    for( IoPin* pin : m_pins ) pin->changeCallBack( el, ch );
}

void IoPort::createPins( Component* comp, QString pins, uint32_t pinMask )
{
    m_numPins = pins.toUInt(0,0);
    if( m_numPins )
    {
        m_pins.resize( m_numPins );

        for( int i=0; i<m_numPins; ++i )
        {
            if( pinMask & 1<<i )
                m_pins[i] = createPin( i, m_name+QString::number(i) , comp );//new IoPin( this, i, m_name+QString::number(i), IoComp );
        }
    }else{
        QStringList pinList = pins.split(",");
        for( QString pinName : pinList )
        {
            IoPin* pin = createPin( m_numPins, m_name+pinName , comp );//new IoPin( this, i, m_name+pinName, IoComp );
            m_pins.emplace_back( pin );
            m_numPins++;
        }
    }
}

IoPin* IoPort::createPin( int i, QString id, Component* comp )
{
    IoPin* pin = new IoPin( 0, QPoint(0,0), comp->getUid()+"-"+id, i, comp, input );
    pin->setOutHighV( 5 );
    return pin;
}

IoPin* IoPort::getPinN( uint8_t i )
{
    if( i >= m_pins.size() ) return NULL;
    return m_pins[i];
}

IoPin* IoPort::getPin( QString pinName )
{
    IoPin* pin = NULL;
    if( pinName.startsWith( m_name ) || pinName.startsWith( m_shortName ) )
    {
        QString pinId = pinName.remove( m_name ).remove( m_shortName );
        int pinNumber = pinId.toInt();
        pin = getPinN( pinNumber );
    }else{
        for( IoPin* ioPin : m_pins )
        {
            QString pid = ioPin->pinId();
            pid = pid.split("-").last().remove( m_name );
            if( pid == pinName )
                return ioPin;
        }
    }
    //if( !pin )
    //    qDebug() << "ERROR: IoPort::getPin NULL Pin:"<< pinName;
    return pin;
}