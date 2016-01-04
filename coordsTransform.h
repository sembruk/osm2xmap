#ifndef COORDS_TRANSFORM_H_INCLUDED
#define COORDS_TRANSFORM_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

extern "C" {
#include <proj_api.h>
}
#include "xmap.h"
#include "common.h"

class TransformData 
: public Georeferencing {
protected:
    projPJ projected_crs;
    projPJ geographic_crs;
public:
    TransformData() {};
    TransformData(const Georeferencing& georef);
};

class Linear
: public Coords {
public:
    Linear(Coords& coords) : Coords(coords) {};
    void translate(const Coords& delta);
    void rotate(double angle); ///< angle in radians!!!
    void scale(double scaleX, double scaleY);
};

class CoordsTransform
: public TransformData, TrueInit {
    Coords& projToMap(Coords& coords);
public:
    CoordsTransform() {};
    CoordsTransform(const Georeferencing& georef) : TransformData(georef), TrueInit(true) {};
    Coords& geographicToMap(Coords& coords);
};

#endif // COORDS_TRANSFORM_H_INCLUDED

