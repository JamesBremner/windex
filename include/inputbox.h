#pragma once
#include "propertygrid.h"
namespace wex
{

/** \brief A popup window where user can edit a set of values.

Usage:
<pre>
    wex::inputbox ib( form );
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
    /// @brief Construct input box modal dialog
    /// @param AppWindow the main application window

    inputbox( gui& AppWindow )
        : myGrid( propertyGrid( this ))
        , myOKButton( maker::make<button>(*this) )
        , myAppWindow( AppWindow )
    {
        windex::get().Add( this );
        text("inputbox");
        move( {100,100,300,300} );
        myGrid.move( { 50,50, 200, 60});
        myGrid.labelWidth( 50 );
        myGrid.bgcolor( 0xFFFFFF );
        myGrid.tabList();
       //myOKButton.move( { 100,300, 50, 40 } );
        myOKButton.text("OK");
        myOKButton.events().click([this]
        {
            //std::cout << "destroying inputbox " << myHandle << "\n";

            // before destroying the window extract values from gui into property attributes
            myGrid.saveValues();

            endModal();
        });
    }
    void clear()
    {
        myGrid.clear();
    }
    wex::property& add(
        const std::string& name,
        const std::string& def )
    {
        ExpandForAdditionalProperty();
        return myGrid.string( name, def );
    }
    wex::property& choice(
        const std::string& name,
        const std::vector<std::string>& choice )
    {
        ExpandForAdditionalProperty();
        return myGrid.choice( name, choice );
    }
    wex::property& check(
        const std::string& name,
        bool def )
    {
        ExpandForAdditionalProperty();
        return myGrid.check( name, def );
    }

    /** Show inputbox and suspend all other windows interactions until this is closed

    The property grid has been expanded to fit the properties as they were added
    this will adjust the form size and OK button size to accomodate the grid
    */
    void show()
    {
        /// adjust for grid size
        auto wh = myGrid.size();
        move( {100,100,wh[0]+100,wh[1]+200} );
        myOKButton.move( { 100,wh[1]+80, 50, 40 } );

        // base class showModal
        gui::showModal(myAppWindow);
    }
    /** @brief  Redirect calls to ShowModal to Show instead
     Inputbox is a popup handling the modal dialog itself
     This should not be called
     for backwards compatibility, it redirects to Show()
     */
    void showModal()
    {
        show();
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
    bool isChecked( const std::string& name )
    {
        return myGrid.isChecked( name );
    }
    void gridWidth( int w )
    {
        myGrid.move( { 50,50, w, 60});
    }
    void labelWidth( int w )
    {
        myGrid.labelWidth( w );
    }
private:
    propertyGrid myGrid;
    button& myOKButton;
    gui& myAppWindow;

    void ExpandForAdditionalProperty()
    {
        myGrid.move( { 50, 50,
                       myGrid.width(),
                       ( myGrid.propCount() + 1 ) * myGrid.propHeight()
                     } );
    }
};
}
