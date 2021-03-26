namespace wex
{
/** \brief A widget which user can drag to change a value.

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
        : gui( parent, "msctls_trackbar32",
               WS_CHILD | WS_OVERLAPPED | WS_VISIBLE |
               TBS_AUTOTICKS | TBS_TRANSPARENTBKGND | TBS_FIXEDLENGTH )
    {
    }
    /** Specify the range values used
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

        SendMessage(
            myHandle,
            TBM_SETRANGE,
            (WPARAM) TRUE,                   // redraw flag
            (LPARAM) MAKELONG(min, max));  // min. & max. positions
    }
    void maximum( int max )
    {
        SendMessage(
            myHandle,
            TBM_SETRANGEMAX,
            true, max );
    }
    /// Change the orientation to be vertical
    void vertical()
    {
        SetWindowLongPtr(
            myHandle,
            GWL_STYLE,
            GetWindowLongPtr( myHandle, GWL_STYLE) |  TBS_VERT );
    }

    /// Position of the slider thumb in range
    int position()
    {
        return SendMessage(
                   myHandle,
                   TBM_GETPOS,
                   (WPARAM) 0, (LPARAM) 0 );
    }
    void position( int pos )
    {
        SendMessage(
            myHandle,
            TBM_SETPOS,
            (WPARAM) true, (LPARAM) pos );
    }
    /// Change thumbnail size ( the arrow that slides along the track )
    void thumbsize( int p )
    {
        SendMessage(
            myHandle,
            TBM_SETTHUMBLENGTH,
            (WPARAM) p,
            (LPARAM) 0 );

    }
};

}
