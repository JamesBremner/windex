#include "propertygrid.h"
namespace wex
{
class inputbox : public window
{
public:
    inputbox( window& parent )
        : W( windex::get())
        , myWindow( W.MakeWindow() )
        , myGrid( propertyGrid( myWindow ))
    {
    }
    void Add(
        const std::string& name,
        const std::string& def )
    {
        myGrid.string( name, def );
    }
    void Show()
    {

    }

    void modal()
    {
        myWindow.show();
    }
private:
    windex& W;
    window& myWindow;
    propertyGrid myGrid;
};
}
