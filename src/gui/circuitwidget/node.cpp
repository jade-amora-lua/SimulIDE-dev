/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
 *                                                                         *
 ***( see copyright.txt file at root folder )*******************************/

#include <QPainter>
#include <QDebug>

#include "node.h"
#include "connector.h"
#include "circuit.h"

#include "doubleprop.h"
#include "pointprop.h"

Node::Node( QObject* parent, QString type, QString id )
    : Component( parent, type, id )
{
    setZValue( 101 );

    m_color = QColor( Qt::black );
    m_isBus = false;

    m_pin.resize( 3 );
    for( int i=0; i<3; i++ )
    {
        m_pin[i] = new Pin( 90*i, QPoint(0,0), id+"-"+uchar(48+i), i, this );
        m_pin[i]->setLength( 0 );
    }
    remPropGroup( "CompGraphic" );
    addPropGroup( { "CompGraphic", {
new PointProp <Component>( "Pos","","",this, &Component::position, &Component::setPosition )
    }} );
}
Node::~Node(){}

void Node::inStateChanged( int rem ) // Called by pin
{
    if     ( rem == 1 ) checkRemove();
    else if( rem == 2 ) // Propagate Is Bus
    {
        for( int i=0; i<3; i++) m_pin[i]->setIsBus( true );
        m_isBus = true;
}   }

void Node::registerEnode( eNode* enode, int n )
{
    for( int i=0; i<3; i++ )
        if( m_pin[i]->conPin() ) m_pin[i]->registerPinsW( enode, n );
}

void Node::checkRemove() // Only remove if there are less than 3 connectors
{
    int con[2] = { 0, 0 };
    int conectors = 0;
    int conecteds = 0;

    for( int i=0; i< 3; i++)
    {
        Connector* co = m_pin[i]->connector();
        if( co )
        {
            Pin* coPin = m_pin[i]->conPin();
            if( coPin->component() == this ) // Connector betwen 2 Pins of this node
            {
                co->setStartPin( NULL );
                co->setEndPin( NULL );
                co->remove();
                continue;
            }
            if( conecteds == 0 ) { conecteds++; con[0] = i; }
            else con[1] = i;
            conectors++;
    }   }
    if( conectors < 3 ) 
    {
        if( conectors == 2 ) joinConns( con[0], con[1] );  // 2 Conn
        else                 m_pin[con[0]]->removeConnector();

        if( !Circuit::self()->deleting() )
        {
            Circuit::self()->nodeList()->removeOne( this );
            Circuit::self()->compMap()->remove( m_id );
            if( this->scene() ) Circuit::self()->removeItem( this );
            this->deleteLater();
        }
        Circuit::self()->addCompState( this, "new", stateAdd );
}   }

void Node::joinConns( int c0, int c1 )
{
    Pin* pin0 = m_pin[c0];
    Pin* pin1 = m_pin[c1];

    Connector* con0 = pin0->connector();
    Connector* con1 = pin1->connector();
    if( !con0 || !con1 ) return;

    if( pin1->conPin() != pin0 )
    {
        Connector* con = new Connector( Circuit::self(), "Connector", "Connector-"+Circuit::self()->newSceneId(), pin0->conPin() );
        Circuit::self()->conList()->append( con );
        Circuit::self()->compMap()->insert( con->getUid(), con );

        QStringList list0 = con0->pointList();
        QStringList list1 = con1->pointList();
        QStringList plist;

        if( pin0 == con0->startPin() ){
            while( !list0.isEmpty() )
            {
                QString p2 = list0.takeLast();
                plist.append(list0.takeLast());
                plist.append(p2);
        }   }
        else while( !list0.isEmpty() ) plist.append( list0.takeFirst() );

        if( pin1 == con1->endPin() ){
            while( !list1.isEmpty() )
            {
                QString p2 = list1.takeLast();
                plist.append(list1.takeLast());
                plist.append(p2);
        }   }
        else while( !list1.isEmpty() ) plist.append( list1.takeFirst() );

        con->setPointList( plist );
        con->closeCon( pin1->conPin() );
        Circuit::self()->addCompState( con, "remove", stateAdd );
        if( this->isSelected() ) con->setSelected( true );
    }
    Circuit::self()->addCompState( con0, "new", stateAdd );
    con0->setStartPin( NULL );
    con0->setEndPin( NULL );
    con0->remove();

    Circuit::self()->addCompState( con1, "new", stateAdd );
    con1->setStartPin( NULL );
    con1->setEndPin( NULL );
    con1->remove();
}

void Node::setHidden(bool hid, bool , bool )
{
    m_hidden = hid;

    for( Pin* pin : m_pin ) pin->setVisible( !hid );
    this->setVisible( !hid );
}

void Node::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    //p->setBrush( Qt::blue );
    //p->drawRect( boundingRect() );

    Component::paint( p, option, widget );
    
    if( m_isBus ) p->drawEllipse( QPointF(0,0), 1.8, 1.8  );
    else          p->drawEllipse( QPointF(0,0), 1.4, 1.4 );
}
