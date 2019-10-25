#pragma once
#include "wex.h"
namespace wex
{
class propertyGrid;

/// A name value pair
class property
{
public:
    property(
        gui* parent,
        const std::string& name,
        const std::string& value )
        : myName( name )
        , myValue( value )
        , W( windex::get())
        , myLabel( W.make<label>(*parent) )
        , myEditbox( W.make<editbox>(*parent) )
        , myCombobox( W.make<choice>(*parent) )
        , myCheckbox( W.make<checkbox>(*parent) )
        , myCategoryExpanded( W.make<checkbox>(*parent) )
        , myLabelWidth( 100 )
        , myType( eType::string )
    {
        myLabel.text( myName );
        myEditbox.text( myValue );
    }
    property(
        gui* parent,
        const std::string& name,
        bool value )
        : myName( name )
        , myValue( std::to_string((int)value ))
        , W( windex::get())
        , myLabel( W.make<label>(*parent) )
        , myEditbox( W.make<editbox>(*parent) )
        , myCombobox( W.make<choice>(*parent) )
        , myCheckbox( W.make<checkbox>(*parent) )
        , myCategoryExpanded( W.make<checkbox>(*parent) )
        , myLabelWidth( 100 )
        , myType( eType::check )
    {
        myLabel.text( myName );
        myCheckbox.check( value );
        myCheckbox.text("");
    }
    property(
        gui* parent,
        const std::string& name,
        const std::vector< std::string >& value )
        : myName( name )
        , myValue( "" )
        , W( windex::get())
        , myLabel( W.make<label>(*parent) )
        , myEditbox( W.make<editbox>(*parent) )
        , myCombobox( W.make<choice>(*parent) )
        , myCheckbox( W.make<checkbox>(*parent) )
        , myCategoryExpanded( W.make<checkbox>(*parent) )
        , myLabelWidth( 100 )
        , myType( eType::choice )
    {
        myLabel.text( myName );
        for( auto& t : value )
        {
            myCombobox.add( t );
        }
    }
    /// construct a category
    property(
        gui* parent,
        const std::string& name )
        : myName( name )
        , W( windex::get())
        , myLabel( W.make<label>(*parent) )
        , myEditbox( W.make<editbox>(*parent) )
        , myCombobox( W.make<choice>(*parent) )
        , myCheckbox( W.make<checkbox>(*parent) )
        , myCategoryExpanded( W.make<checkbox>(*parent) )
        , myType( eType::category )
    {
        myCategoryExpanded.text( myName );
        myCategoryExpanded.plus();
        myCategoryExpanded.check();
        myCategoryExpanded.events().clickPropogate();
    }
    void move( const std::vector<int>& r )
    {
        std::vector<int> rl( r );
        rl[2] = myLabelWidth;
        myLabel.move( rl );

        std::vector<int> re( r );
        re[0] += myLabelWidth;
        re[2] -= myLabelWidth;
        switch( myType )
        {
        case eType::string:
            myCategoryExpanded.move({0,0,0,0});
            myEditbox.move( re );
            break;
        case eType::choice:
            re[3] *= myCombobox.count();
            myCombobox.move( re );
            break;
        case eType::check:
            myCheckbox.move( re );
            break;
        case eType::category:
            myLabel.move({0,0,0,0});        // keep unused label out of the way
            myCategoryExpanded.move( r );
            break;
        }
    }
    void labelWidth( int w )
    {
        myLabelWidth = w;
    }
    void bgcolor( int color )
    {
        myLabel.bgcolor( color );
    }
    void show( bool f = true )
    {
        myLabel.show( f );
        switch( myType )
        {
        case eType::string:
            myEditbox.show( f );
            break;
        case eType::choice:
            myCombobox.show( f );
            break;
        case eType::check:
            myCheckbox.show( f );
            break;
        case eType::category:
            myLabel.show( false );        // keep unused label out of the way
            myCategoryExpanded.show( f );
            break;
        }
    }
    /** Update ( redraw ) property child widgets

    Some widgets update nicely when the window containing the property grid resizes
    Others, most significantly labels, do not.  So application code should
    handle containing window resizes by calling propertyGrid::update()
    which will call this method on all properties.
    */
    void update()
    {
        myLabel.update();
        myCheckbox.update();
        myCategoryExpanded.update();
    }

    const std::string& name() const
    {
        return myName;
    }
    const std::string value() const
    {
        switch( myType )
        {
        case eType::string:
            return myEditbox.text();

        case eType::choice:
            return myCombobox.SelectedText();

        case eType::check:
            return std::to_string( (int) myCheckbox.isChecked() );
            break;

        case eType::category:
            break;
        }
        return std::string("");
    }

    // copy value from gui into myValue attribute
    void saveValue()
    {
        switch( myType )
        {
        case eType::string:
            myValue = myEditbox.text();
            break;
        case eType::choice:
            myValue = myCombobox.SelectedText();
            break;
        case eType::check:
            myValue = std::to_string( (int) myCheckbox.isChecked() );
            break;
        case eType::category:
            break;
        }
    }

    // get myValue attribute
    const std::string savedValue() const
    {
        return myValue;
    }

    bool isCategory() const
    {
        return myType == eType::category;
    }
    bool isExpanded() const
    {
        if( ! isCategory() )
            return false;
        return myCategoryExpanded.isChecked();
    }
    void expand( bool f )
    {
        myCategoryExpanded.check( f );
    }
private:
    std::string myName;
    std::string myValue;
    wex::windex& W;
    wex::label& myLabel;
    wex::editbox& myEditbox;
    wex::choice& myCombobox;
    wex::checkbox& myCheckbox;
    wex::checkbox& myCategoryExpanded;
    int myLabelWidth;
    enum class eType
    {
        string,
        choice,
        check,
        category
    }
    myType;
};
/// A grid of properties
class propertyGrid : public gui
{
public:
    propertyGrid( gui* parent )
        : gui( parent )
        , myHeight( 25 )
        , myWidth( 300 )
        , myLabelWidth( 100 )
        , myBGColor( 0xc8c8c8)
    {
        text("PG");
        scroll();

        events().click([this]
        {
            visible();
        });
    }
    void string(
        const std::string& name,
        const std::string& value )
    {
        property P( this, name, value );
        CommonConstruction( P );
    }
    void choice(
        const std::string& name,
        const std::vector< std::string >& choice )
    {
        property P( this, name, choice );
        CommonConstruction( P );
    }
    void check(
        const std::string& name,
        bool f )
    {
        property P( this, name, f );
        CommonConstruction( P );
    }
    void category(
        const std::string& name )
    {
        property P( this, name );
        CommonConstruction( P );
    }
    void expand(
        const std::string name,
        bool fexpand = true )
    {
        for( auto& p : myProperty )
        {
            if( p.name() == name &&
                    p.isCategory() )
            {
               p.expand( fexpand );
               visible();
               return;
            }
        }
    }
    void move( const std::vector<int>& r )
    {
        gui::move( r );
        myWidth = r[2];
    }
    void labelWidth( int w )
    {
        myLabelWidth = w;
    }

    /// force every property to redraw its label
    void update()
    {
        for( auto& p : myProperty )
            p.update();
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

    /// get value in editbox of property with name
    const std::string value( const std::string& name )
    {
        std::cout << "PG value " << name << "\n";
        property* p = find( name );
        if( ! p )
        {
            static std::string null;
            return null;
        }
        std::string v = p->value();
        return p->value();
    }

    /// save values in all property textboxes in the property's myValue attribute
    void saveValues()
    {
        for( auto& p : myProperty )
        {
            p.saveValue();
        }
    }

    int propHeight() const
    {
        return myHeight;
    }
    int width() const
    {
        return myWidth;
    }
private:
    std::vector< property > myProperty;
    int myHeight;               // height of a single property
    int myWidth;
    int myLabelWidth;
    int myBGColor;

    void CommonConstruction( property& P )
    {
        P.labelWidth( myLabelWidth );
        P.bgcolor( myBGColor );
//        P.move(
//        {
//            0, (int)myProperty.size() * myHeight,
//            myWidth, myHeight
//        } );
        myProperty.push_back( P );

        scrollRange(
            myWidth,
            ((int)myProperty.size()+1) * myHeight);

        visible();
    }
    /** Show properties when category is expanded or collapsed
    */
    void visible(  )
    {
        for( auto& P : myProperty)
            P.show( false );

//        RedrawWindow(
//                     myHandle,
//                     NULL, NULL,
//                     RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN );

        bool expanded = true;
        int index = 0;
        for( auto& P : myProperty)
        {
            if( P.isCategory() )
            {
                //std::cout << "cat " << P.name()  << " at " << index << "\n";
                P.move( { 0, index * myHeight, myWidth, 2 * myHeight } );     // category always visible
                P.show();
                index += 2;             // display takes two rows
                expanded = P.isExpanded();       // control visibility of contained properties
            }
            else if( expanded )
            {
                //std::cout << "show " << P.name() <<" at "<< index << "\n";
                P.move( { 0, index * myHeight, myWidth, myHeight } );     // property is visible
                P.show();
                index++;                // displays in one row
            }
            else
            {
                //std::cout << "hide " << P.name() << "\n";
                P.move( { 0,0,0,0 });    // invisible property
                P.show( false );
            }
        }
        scrollRange(
            myWidth,
            index * myHeight);
        update();
    }
};
}
