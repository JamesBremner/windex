#include "wex.h"
namespace wex
{
class property
{
    public:
    property(
        wex::window& parent,
        const std::string& name,
        const std::string& value )
        : myName( name )
        , myValue( value )
        , W( wex::windex::get())
        , myLabel( W.make<label>(parent) )
        , myEditbox( W.make<editbox>(parent) )
    {
        myLabel.text( myName );
        myEditbox.text( myValue );
    }
    void move( const std::vector<int>& r )
    {
        myLabel.move( r );
        std::vector<int> re( r );
        re[0] += 70;
        re[2] -= 70;
        myEditbox.move( re );
    }
private:
    std::string myName;
    std::string myValue;
    wex::windex& W;
    wex::label& myLabel;
    wex::editbox& myEditbox;
};
}
