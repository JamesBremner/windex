#pragma once
#include "propertygrid.h"
namespace wex
{

/** \brief A popup window where user can edit a set of values.

Usage:
<pre>
    wex::inputbox ib;
    ib.add("A","72");
    ib.add("B","4600");
    ib.choice("Choose", { "X", "Y"} );
    ib.showModal();

    std::string msg =
        "A is " + ib.value("A") +
        ", B is " + ib.value("B") +
        ", choice is " + ib.value("Choose");
    msgbox(
        form,
        msg );
</pre>
*/
class inputbox : public gui
{
public:
    inputbox()
        : myGrid( propertyGrid( this ))
        , myOKButton( maker::make<button>(*this) )
    {
        windex::get().Add( this );
        text("inputbox");
        move( {100,100,300,300} );
        myGrid.move( { 50,50, 200, 60});
        myGrid.labelWidth( 50 );
        myGrid.bgcolor( 0xFFFFFF );
        myGrid.tabList();
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
        myGrid.move( { 50,50, myGrid.width(), myGrid.propCount() * myGrid.propHeight() } );
    }
    void choice(
        const std::string& name,
        const std::vector<std::string>& choice )
    {
        myGrid.choice( name, choice );
        myGrid.move( { 50,50, myGrid.width(), myGrid.propCount() * myGrid.propHeight() } );
    }
    void check(
        const std::string& name,
        bool def )
    {
        myGrid.check( name, def );
        myGrid.move( { 50,50, myGrid.width(), myGrid.propCount() * myGrid.propHeight() } );
    }

    void modal()
    {
        showModal();
    }
    /** get value saved in property attribute
        @param[in] name of property
        @return property value, if property not found "property not found"
    */
    std::string value ( const std::string& name )
    {
        auto p = myGrid.find( name );
        if( p == nullptr )
            return "property not found";
        return p->savedValue();
    }
    void gridWidth( int w )
    {
        std::cout << "inputbox::gridWidth " << w << "\n";
        myGrid.move( { 50,50, w, 60});
    }
    void labelWidth( int w )
    {
        myGrid.labelWidth( w );
    }
private:
    propertyGrid myGrid;
    button& myOKButton;
};
}
