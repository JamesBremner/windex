namespace wex
{
/** \brief A widget which user can drag to change a value

<pre>
    // construct top level window
    gui& form = wex::windex::topWindow();
    form.move({ 50,50,500,400});
    form.text("Slider demo");

    // construct labels to display values when sliders are moved
    wex::label& label = wex::make<wex::label>(form);
    label.move( 200, 200, 100,30 );
    label.text("");
    wex::label& vlabel = wex::make<wex::label>(form);
    vlabel.move( 200, 240, 100,30 );
    vlabel.text("");

    // construct horizontal slider
    wex::slider& S = wex::make<wex::slider>( form );
    S.move({ 50,50,400,50});
    S.range( 0, 100 );
    S.text("horiz slider");
    S.events().slid([&](int pos)
    {
        label.text("horiz value: " + std::to_string( pos ));
        label.update();
    });

    // construct vertical slider
    wex::slider& V = wex::make<wex::slider>( form );
    V.move({ 50,100,50,400});
    V.range( 0, 10 );
    V.vertical();
    V.events().slid([&](int pos)
    {
        vlabel.text("vert value: " + std::to_string( pos ));
        vlabel.update();
    });

    form.show();
</pre>
*/

class slider : public gui
{
public:
    slider( gui* parent )
        : gui( parent )
        , myPosition( 50 )
        , myMin( 0 )
        , myMax( 100 )
        , fsliding( false )
        , ftracking( false )
        , fvertical( false )
    {
        events().click([this]
        {
            focus();
            RECT r;
            GetClientRect( myHandle, &r );
            int height = r.bottom - (r.bottom - r.top ) * myPosition / myMax;
            auto m = getMouseStatus();
            if( abs(m.y - height) > 10  )
                return;
            //std::cout << "slider2 click\n";
            fsliding = true;
        });
        events().mouseMove([this](sMouse& m )
        {
            if( ! ftracking )
            {
                // first mouse movement in slider
                // start tracking mouse movement
                ftracking = true;

                // generate event when mose leaves
                TRACKMOUSEEVENT s;
                s.cbSize = sizeof( s );
                s.hwndTrack = myHandle;
                s.dwFlags = TME_LEAVE;
                TrackMouseEvent( & s );

                // run mouse enter event handler
                events().onMouseEnter();
            }
            if( ! fsliding )
            {
                return;
            }
            RECT r;
            GetClientRect( myHandle, &r );
//                        std::cout << "m.y " << m.y
//                <<"  "<< r.bottom <<" " << r.top <<" ";
            if( fvertical )
            {
                if( m.y > r.bottom || m.y < r.top )
                    return;
                myPosition =  myMax * ( r.bottom - m.y ) / ( r.bottom - r.top );
            }
            else
            {
                if( m.x < r.left || m.x > r.right )
                    return;
                myPosition = myMax * ( m.x - r.left ) / ( r.right - r.left );
            }
            if( myPosition > myMax )
                myPosition = myMax;
//            std::cout << myPosition << " ";
            update();
            events().onSlid( myPosition );
        });
        events().mouseUp([this]
        {
            fsliding = false;

        });
    }
    /** Specify the range of values used
    @param[in] min
    @param[in] max

    The values must all be positive,
    otherwise a runtime_error exception is thrown
    */
    void range( int min, int max )
    {
        if( min < 0 )
            throw std::runtime_error(
                "wex::slider positions must be positive");
        if( max < min )
            throw std::runtime_error(
                "wex::slider bad range parameters");

        myMin = min;
        myMax = max;
        myPosition = ( myMax + myMin ) / 2;
    }
    void stopTracking()
    {
        ftracking = false;
    }
    void draw( PAINTSTRUCT& ps )
    {
        RECT r;
        GetClientRect( myHandle, &r );

        // track
        wex::shapes s( ps );
        if( ! myfEnabled )
            s.color( 0xAAAAAA );
        if( fvertical )
        {
            int center = r.left+(r.right-r.left)/2;
            s.line( {center-1,r.top+5,center-1,r.bottom-5});
            s.line( {center+1,r.top+5,center+1,r.bottom-5});
        }
        else
        {
            int center = r.top+(r.bottom-r.top)/2;
            s.line({r.left+5,center-1,r.right-5,center-1});
            s.line({r.left+5,center+1,r.right-5,center+1});
        }

        // thumbnail
        if( myfEnabled )
            SelectObject(ps.hdc, GetStockObject(BLACK_BRUSH));
        else
            SelectObject(ps.hdc, GetStockObject(GRAY_BRUSH));
        if( fvertical )
        {
            int height = r.bottom - (r.bottom - r.top ) * myPosition / myMax;
            RoundRect(ps.hdc,
                      r.left, height -4,
                      r.right, height + 4,
                      5, 5);
        }
        else
        {
            int height = r.left + (r.right - r.left ) * myPosition / myMax;
            RoundRect(ps.hdc,
                      height - 4, r.top,
                      height + 4, r.bottom,
                      5, 5);
        }
    }
    void vertical(bool f = true)
    {
        fvertical = f;
    }

    double position() const
    {
        return myPosition;
    }
    void position( double v )
    {
        myPosition = v;
    }
    void maximum( double max )
    {
        myMax = max;
    }
private:
    double myPosition;
    double myMin;
    double myMax;
    bool fsliding;
    bool ftracking;
    bool fvertical;
};

}
