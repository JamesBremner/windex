namespace wex {
/** \brief Widget to display a value on a circular clock face
*/
class gauge : public gui
{
public:
    gauge( gui* parent )
        : gui( parent )
        , myMax( 10 )
    {

    }
    void range( int max )
    {
        myMax = max;
    }

    void value( double v )
    {
        myValue = v;
    }
protected:
    virtual void draw( PAINTSTRUCT& ps )
    {
        int w = ps.rcPaint.right - ps.rcPaint.left;
        int h = ps.rcPaint.bottom - ps.rcPaint.top;
        int x0 = ps.rcPaint.left + w / 2;
        int y0 = ps.rcPaint.top + h / 2;
        shapes S( ps );
        S.circle( x0, y0, w / 2 );
        int inc = myMax / 10;
        int theta = 30;
        double r = w;
        if( h <   w )
            r = h;
        r *= 0.95;
        r /= 2;
        std::cout << "radius  " << r << "\n";
        for( int k = 1; k <= 10; k++ )
        {
            theta += 20;
            double rads = 0.0174533 * theta;
            int x = x0 - sin( rads ) * r * 0.8;
            int y = y0 + cos( rads ) * r * 0.8;
            S.text( std::to_string( k * inc ), { x,y,30,30});
            S.line( { x, y,
                   (int)(x0 - sin( rads ) * r),
                   (int)(y0 + cos( rads ) * r) });

        }
        theta = 30 + 200 * myValue / myMax;
        double rads = 0.0174533 * theta;
        S.penThick( 3 );
        S.color( 0 );
        S.line( { x0,y0,
               (int)( x0 - sin( rads ) * r * 0.9),
               (int)( y0 + cos( rads ) * r * 0.9 )});
    }
private:

    int myMax;
    double myValue;
};
}

