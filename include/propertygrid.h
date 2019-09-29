#include <memory>
#include "wex.h"
namespace wex
{
/// A name value pair
class property
{
public:
    property(
        window& parent,
        const std::string& name,
        const std::string& value )
        : myName( name )
        , myValue( value )
        , W( windex::get())
        , myLabel( W.make<label>(parent) )
        , myEditbox( W.make<editbox>(parent) )
        , myLabelWidth( 100 )
    {
        myLabel.text( myName );
        myEditbox.text( myValue );
    }
    void move( const std::vector<int>& r )
    {
        myLabel.move( r );
        std::vector<int> re( r );
        re[0] += myLabelWidth;
        re[2] -= myLabelWidth;
        myEditbox.move( re );
    }
    void labelWidth( int w )
    {
        myLabelWidth = w;
    }
    void bgcolor( int color )
    {
        myLabel.bgcolor( color );
    }
    const std::string& name() const
    {
        return myName;
    }
    const std::string& value() const
    {
        return myValue;
    }
private:
    std::string myName;
    std::string myValue;
    wex::windex& W;
    wex::label& myLabel;
    wex::editbox& myEditbox;
    int myLabelWidth;
};
/// A grid of properties
class propertyGrid
{
public:
    propertyGrid( window& parent )
        : myParent( parent )
        , myHeight( 30 )
        , myWidth( 300 )
        , myLabelWidth( 100 )
        , myX( 10 )
        , myY( 10 )
        , myBGColor( 0xc8c8c8)
    {

    }
    void string(
        const std::string& name,
        const std::string& value )
    {
        property P( myParent, name, value );
        P.labelWidth( myLabelWidth );
        P.bgcolor( myBGColor );
        P.move( { myX, myY+(int)myProperty.size() * myHeight,
                    myWidth, myHeight } );
        myProperty.push_back( P );
    }
    void move( const std::vector<int>& r )
    {
        myX = r[0];
        myY = r[1];
        myWidth = r[2];
    }
    void labelWidth( int w )
    {
        myLabelWidth = w;
    }
    void bgcolor( int color )
    {
        myBGColor = color;
    }

    /// get pointer to property with name
    property* find( const std::string& name )
    {
        for( auto& p : myProperty )
        {
            if( p.name() == name )
                return &p;
        }
        return nullptr;
    }

    /// get value of property with name
    const std::string& value( const std::string& name )
    {
        property* p = find( name );
        if( ! p )
            return "";
        return p->value();
    }
private:
    std::vector< property > myProperty;
    window& myParent;
    int myHeight;               // height of a single property
    int myWidth;
    int myLabelWidth;
    int myX, myY;
    int myBGColor;
};
}
