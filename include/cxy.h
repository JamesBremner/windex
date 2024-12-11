
#pragma once
#include <cfloat>
#include <cmath>

/// @brief 2D point or vector
class cxy
{

public:
    double x;
    double y;

    cxy()
        : x(-DBL_MAX),
          y(-DBL_MAX)
    {
    }
    cxy(double X, double Y)
        : x(X), y(Y)
    {
    }
    cxy(const cxy &xy)
        : x(xy.x), y(xy.y)
    {
    }
    /// @brief vector from this point to other
    /// @param other
    /// @return
    cxy vect(const cxy &other) const
    {
        cxy v;
        v.x = other.x - x;
        v.y = other.y - y;
        return v;
    }

    /// @brief distance squared from this point to other
    /// @param other
    /// @return
    double dist2(const cxy &other) const
    {
        cxy v = vect(other);
        return v.x * v.x + v.y * v.y;
    }

    // closest point on line to this point, fraction ( point = end1 + t * (end2-end1) )
    // return t
    double tclosest(
        const cxy &end1,
        const cxy &end2) const
    {
        cxy AB = end1.vect(end2);
        cxy AP = end1.vect(*this);
        double lAB = AB.x * AB.x + AB.y * AB.y;
        return (AB.x * AP.x + AB.y * AP.y) / lAB;
    }

    // closest point on line segment to this point
    cxy closest(
        const cxy &end1,
        const cxy &end2) const
    {
        double t = tclosest(
            end1, end2);
        if (t < 0)
            t = 0;
        if (t > 1)
            t = 1;
        cxy ret(end1.x + t * (end2.x - end1.x),
                end1.y + t * (end2.y - end1.y));
        return ret;
    }

    /// @brief distance squared from this point to nearest point on line segment
    /// @param end1 line segment endpoint
    /// @param end2 line segment endpoint
    /// @return
    double dis2toline(
        const cxy &end1,
        const cxy &end2) const
    {
        double t = tclosest(end1, end2);
        if (t < 0)
            t = 0;
        if (t > 1)
            t = 1;
        cxy AB = end1.vect(end2);
        cxy closest(
            end1.x + t * AB.x,
            end1.y + t * AB.y);
        return dist2(closest);
    }

    /// true if point inside polygon
    bool isInside(const std::vector<cxy> &polygon) const
    {
        int c = 0;
        std::vector<cxy>::const_iterator j = polygon.end() - 1;
        for (std::vector<cxy>::const_iterator i = polygon.begin();
             i != polygon.end(); j = i++)
        {
            if (((i->y > y) != (j->y > y)) &&
                (x < (j->x - i->x) * (y - i->y) / (j->y - i->y) + i->x))
                c = !c;
        }
        return (c == 1);
    }

    static cxy enclosingWidthHeight(const std::vector<cxy> &polygon)
    {
        cxy max, min;
        for (const cxy &p : polygon)
        {
            if (p.x > max.x)
                max.x = p.x;
            if (p.y > max.y)
                max.y = p.y;
            if (p.x < min.x)
                min.x = p.x;
            if (p.y < min.y)
                min.y = p.y;
        }
        static cxy ret;
        ret.x = max.x - min.x;
        ret.y = max.y - min.y;
        return ret;
    }

    /** true if line segments intersect
        @param[out] p point of intersection
        @param[in] a,b line segment
        @param[in] c,d line segment
        @return true if line segments intersect
    */
    static bool isIntersection(
        cxy &p,
        const cxy &a, const cxy &b,
        const cxy &c, const cxy &d)
    {

        /*
        https://www.topcoder.com/thrive/articles/Geometry%20Concepts%20part%202:%20%20Line%20Intersection%20and%20its%20Applications

    Ax+By=C

    A = y2-y1
    B = x1-x2
    C = Ax1+By1

        */
        double A1 = b.y - a.y;
        double B1 = a.x - b.x;
        double C1 = A1 * a.x + B1 * a.y;
        double A2 = d.y - c.y;
        double B2 = c.x - d.x;
        double C2 = A2 * c.x + B2 * c.y;

        /**
         * intersection point
         *
         * double det = A1 * B2 - A2 * B1
        if (det == 0) {
          //Lines are parallel
        } else {
          double x = (B2 * C1 - B1 * C2) / det
          double y = (A1 * C2 - A2 * C1) / det
        }
         *
         */
        double det = A1 * B2 - A2 * B1;
        if (fabs(det) < 0.0001)
            return false;
        else
        {
            // intersection point of infinite lines
            p.x = (B2 * C1 - B1 * C2) / det;
            p.y = (A1 * C2 - A2 * C1) / det;

            // check segments intersect
            if (!(std::min(a.x, b.x) <= p.x && p.x <= std::max(a.x, b.x)))
                return false;
            if (!(std::min(a.y, b.y) <= p.y && p.y <= std::max(a.y, b.y)))
                return false;
            if (!(std::min(c.x, d.x) <= p.x && p.x <= std::max(c.x, d.x)))
                return false;
            if (!(std::min(c.y, d.y) <= p.y && p.y <= std::max(c.y, d.y)))
                return false;

            return true;
        }
    }

    static bool isIntersect(cxy &p1, cxy &q1, cxy &p2, cxy &q2);

    /// @brief angle between line segments, radians
    /// @param a,b line segment
    /// @param c,d line segment
    /// @return angle in radians

    static double angle(
        const cxy &a, const cxy &b,
        const cxy &c, const cxy &d)
    {
        cxy v1 = a.vect(b);
        cxy v2 = c.vect(d);
        double dot = v1.x * v2.x + v1.y * v2.y;
        double det = v1.x * v2.y - v1.y * v2.x;
        return atan2(det, dot);
    }

    /// @brief clockwise turn going from a to b to c, radians
    /// @param a
    /// @param b
    /// @param c
    /// @return angle in radians

    static double clockwise(
        const cxy &a,
        const cxy &b,
        const cxy &c)
    {
        double ang = cxy::angle(
            b, a,
            b, c);
        // go around the other way
        if (ang < 0)
            ang += 6.28;
        return ang;
    }

    bool isValid() const
    {
        return ((x > -DBL_MAX + 1) && (y > -DBL_MAX + 1));
    }
    void invalidate()
    {
        x = -DBL_MAX;
        y = -DBL_MAX;
    }
    void zoom(float ratio)
    {
        x *= ratio;
        y *= ratio;
    }
    bool operator==(const cxy &other) const
    {
        return x == other.x && y == other.y;
    }
    cxy operator+(const cxy &other) const
    {
        cxy ret(*this);
        ret.x += other.x;
        ret.y += other.y;
        return ret;
    }
    cxy operator-(const cxy &other) const
    {
        cxy ret(*this);
        ret.x -= other.x;
        ret.y -= other.y;
        return ret;
    }
    cxy &operator*=(float s)
    {
        x *= s;
        y *= s;
        return *this;
    }
    cxy &operator*=(const cxy &other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }
    cxy &operator+=(const cxy &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    cxy &operator/=(float s)
    {
        x /= s;
        y /= s;
        return *this;
    }
    friend std::ostream &operator<<(std::ostream &os, cxy p)
    {
        os << "(" << p.x << "," << p.y << ")";
        return os;
    }
};
/// @brief 3D point or vector
class cxyz
{
public:
    double x, y, z;

    cxyz()
        : x(-DBL_MAX),
          y(-DBL_MAX),
          z(-DBL_MAX)
    {
    }
    cxyz(double X, double Y, double Z)
        : x(X), y(Y), z(Z)
    {
    }

    /// @brief vector from this point to other
    /// @param other
    /// @return
    cxyz vect(const cxyz &other) const
    {
        return cxyz(
            other.x - x,
            other.y - y,
            other.z - z);
    }

    static cxyz plane(
        const cxyz &p0,
        const cxyz &p1,
        const cxyz &p2)
    {
        cxyz p01 = p0.vect(p1);
        cxyz p02 = p0.vect(p2);
        return cxyz(
            p0.x + p01.x + p02.x,
            p0.y + p01.y + p02.y,
            p0.z + p01.z + p02.z);
    }

    cxyz cross(const cxyz &other)
    {
        return cxyz(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.z);
    }

    double dot(const cxyz &other)
    {
        return x * other.x +
               y * other.y +
               z * other.z;
    }

    /// @brief intersection point between line and triangle
    /// @param la
    /// @param lb
    /// @param plane
    /// @return intersection point
    /// https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection#Parametric_form

    static cxyz intersectLineTriangle(
        const cxyz &la, const cxyz &lb,
        const cxyz &p0, const cxyz &p1, const cxyz &p2)
    {
        cxyz crossall = p0.vect(p1).cross(p0.vect(p2));
        cxyz crossu = p0.vect(p2).cross(lb.vect(la));
        cxyz crossv = lb.vect(la).cross(p0.vect(p2));
        cxyz lap0(la.x - p0.x, la.y - p0.y, la.z - p0.x);
        double divisor = lb.vect(la).dot(crossall);
        double t = crossall.dot(lap0) / divisor;
        double u = crossu.dot(lap0) / divisor;
        double v = crossv.dot(lap0) / divisor;

        // check that line intersects triangle
        if (t >= 0 && t <= 1)
            if (u >= 0 && u <= 1)
                if (v >= 0 && v <= 1)
                    if (u + v <= 1)

                        return cxyz(
                            la.x + t * (lb.x - la.x),
                            la.y + t * (lb.y - la.y),
                            la.z + t * (lb.z - la.z));

        return cxyz();
    }
};