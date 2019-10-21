#pragma once
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
        , myCombobox( W.make<choice>(parent) )
        , myCheckbox( W.make<checkbox>(parent) )
        , myLabelWidth( 100 )
        , myType( eType::string )
    {
        myLabel.text( myName );
        myEditbox.text( myValue );
    }
    property(
        window& parent,
        const std::string& name,
        bool value )
        : myName( name )
        , myValue( std::to_string((int)value ))
        , W( windex::get())
        , myLabel( W.make<label>(parent) )
        , myEditbox( W.make<editbox>(parent) )
        , myCombobox( W.make<choice>(parent) )
        , myCheckbox( W.make<checkbox>(parent) )
        , myLabelWidth( 100 )
        , myType( eType::check )
    {
        myLabel.text( myName );
        myCheckbox.check( value );
        myCheckbox.text("");
    }
    property(
        window& parent,
        const std::string& name,
        const std::vector< std::string >& value )
        : myName( name )
        , myValue( "" )
        , W( windex::get())
        , myLabel( W.make<label>(parent) )
        , myEditbox( W.make<editbox>(parent) )
        , myCombobox( W.make<choice>(parent) )
        , myCheckbox( W.make<checkbox>(parent) )
        , myLabelWidth( 100 )
        , myType( eType::choice )
    {
        myLabel.text( myName );
        for( auto& t : value )
        {
            myCombobox.add( t );
        }
    }
    property(
        window& parent,
        const std::string& name )
        : myName( name )
        , W( windex::get())
        , myLabel( W.make<label>(parent) )
        , myEditbox( W.make<editbox>(parent) )
        , myCombobox( W.make<choice>(parent) )
        , myCheckbox( W.make<checkbox>(parent) )
        , myType( eType::category )
    {
        myLabel.text( myName );
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
            myLabel.move( r );
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
    }

    /// force label to redraw
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
private:
    std::string myName;
    std::string myValue;
    wex::windex& W;
    wex::label& myLabel;
    wex::editbox& myEditbox;
    wex::choice& myCombobox;
    wex::checkbox& myCheckbox;
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
        CommonConstruction( P );
    }
    void choice(
        const std::string& name,
        const std::vector< std::string >& choice )
    {
        property P( myParent, name, choice );
        CommonConstruction( P );
    }
    void check(
        const std::string& name,
        bool f )
    {
        property P( myParent, name, f );
        CommonConstruction( P );
    }
    void category(
        const std::string& name )
    {
        property P( myParent, name );
        CommonConstruction( P );
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
private:
    std::vector< property > myProperty;
    window& myParent;
    int myHeight;               // height of a single property
    int myWidth;
    int myLabelWidth;
    int myX, myY;
    int myBGColor;

    void CommonConstruction( property& P )
    {
        P.labelWidth( myLabelWidth );
        P.bgcolor( myBGColor );
        P.move(
        {
            myX, myY+(int)myProperty.size() * myHeight,
            myWidth, myHeight
        } );
        myProperty.push_back( P );
    }
};
}
