#define CATCH_CONFIG_MAIN
#include "../catch.hpp"

#include "../Averager.h"
// #include <whiteboard/builtinTypes/Vector3D.h>

// A class for testing the averager with more complex objects
template <typename T>
class Vector3D {
public:
    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(T xVal, T yVal, T zVal) : x(xVal), y(yVal), z(zVal) {}
    Vector3D<T>& operator+=(const Vector3D<T>& other) {
        this->x += other.x;
        this->y += other.y;
        this->z += other.z;
    }
    template <typename U>
    Vector3D<T> operator/(U value) const {
        return Vector3D<T>(this->x / value, this->y / value, this->z / value);
    }

    bool operator==(const Vector3D<T>& other) const {
        return &other == this ||
            (this->x == other.x && this->y == other.y && this->z == other.z);
    }

private:
    T x;
    T y;
    T z;
};

TEST_CASE( "Averager works with ints" ) {
    Averager<int> averager;

    REQUIRE( averager.getAccumulatorValue() == 0 );
    REQUIRE( averager.getAverage() == 0 );

    averager.addValue(10);
    averager.addValue(20);
    averager.addValue(30);

    REQUIRE( averager.getAccumulatorValue() == 60 );
    REQUIRE( averager.getAverage() == 20 );
    REQUIRE( averager.getAccumulatorValue() == 60 );

    averager.reset();
    REQUIRE( averager.getAccumulatorValue() == 0 );
    REQUIRE( averager.getAverage() == 0 );

    averager.addValue(100);
    REQUIRE( averager.getAverage() == 100 );
}

TEST_CASE( "Averager works with 3D vectors" ) {
    Averager<Vector3D<double>> averager;

    REQUIRE( averager.getAverage() == Vector3D<double>() );

    averager.addValue(Vector3D<double>(10.0, 20.0, 30.0));
    averager.addValue(Vector3D<double>(30.0, 20.0, 10.0));
    averager.addValue(Vector3D<double>(20.0, 20.0, 20.0));

    REQUIRE( averager.getAccumulatorValue() == Vector3D<double>(60.0, 60.0, 60.0) );
    REQUIRE( averager.getAverage() == Vector3D<double>(20.0, 20.0, 20.0) );

    averager.reset();

    REQUIRE( averager.getAverage() == Vector3D<double>() );

    averager.addValue(Vector3D<double>(100.0, -100.0, 0.0));

    REQUIRE( averager.getAverage() == Vector3D<double>(100.0, -100.0, 0.0) );
}
