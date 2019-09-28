#include <memory>
#include "wex.h"
namespace wex
{
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
private:
    std::string myName;
    std::string myValue;
    wex::windex& W;
    wex::label& myLabel;
    wex::editbox& myEditbox;
    int myLabelWidth;
};
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
    {

    }
    void string(
        const std::string& name,
        const std::string& value )
    {
        property P( myParent, name, value );
        P.labelWidth( myLabelWidth );
        P.move( { myX, myY+(int)myProperty.size() * myHeight,
                    myWidth, myHeight } );
        myProperty.push_back( P );
    }
    void labelWidth( int w )
    {
        myLabelWidth = w;
    }
private:
    std::vector< property > myProperty;
    window& myParent;
    int myHeight;               // height of a single property
    int myWidth;
    int myLabelWidth;
    int myX, myY;
};
}
