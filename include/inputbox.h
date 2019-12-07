#include "propertygrid.h"
namespace wex
{

/// A popup window where user can edit a set of values
class inputbox : public gui
{
public:
    inputbox( gui& parent )
        : myGrid( propertyGrid( this ))
        , myOKButton( maker::make<button>(*this) )
    {
        windex::get().Add( this );
        text("inputbox");
        move({100,100,300,300});
        myGrid.move( { 50,50, 200, 60});
        myGrid.labelWidth( 50 );
        myGrid.bgcolor( 0xFFA0A0 );
        myOKButton.move( { 100,200, 50, 40 } );
        myOKButton.text("OK");
        myOKButton.events().click([this]
        {
            //std::cout << "destroying inputbox " << myHandle << "\n";

            // before destroying the window extract values from gui into property attributes
            myGrid.saveValues();

            myfModal = false;
            DestroyWindow(myHandle);
        });
    }
    void add(
        const std::string& name,
        const std::string& def )
    {
        myGrid.string( name, def );
        myGrid.move( { 50,50, 200, myGrid.propCount() * myGrid.propHeight() } );
    }
    void choice(
        const std::string& name,
        const std::vector<std::string>& choice )
    {
        myGrid.choice( name, choice );
        myGrid.move( { 50,50, 200, myGrid.propCount() * myGrid.propHeight() } );
    }

    void modal()
    {
        showModal();
    }
    /// get value saved in property attribute
    std::string value ( const std::string& name )
    {
        auto p = myGrid.find( name );
        return p->savedValue();
    }
private:
    propertyGrid myGrid;
    button& myOKButton;
};
}
