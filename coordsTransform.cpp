#include "coordsTransform.h"

Georeferencing::Georeferencing(XmlElement& root, const Coords& geographic_ref_point) {
    XmlElement georeferencingNode = root.getChild("georeferencing");
    mapScale = 1000.0 / georeferencingNode.getAttribute<double>("scale");

    projectedCrsDesc = "";
    geographicCrsDesc = "+proj=latlong +datum=WGS84";

    if (!(projected_crs = pj_init_plus(projectedCrsDesc.c_str())) ) {
        throw Error("projected coordinate system init failed!");
    }
    if (!(geographic_crs = pj_init_plus(geographicCrsDesc.c_str())) ) {
        throw Error("geographic coordinate system init failed!");
    }
#ifdef DEBUG
    info("Loaded georeferencing:");
    info("\tmapScale" + std::to_string(mapScale));
    info("\tgrivation " + std::to_string(grivation));
    info("\tmapRefPoint " + std::to_string(mapRefPoint.X()) + " " + std::to_string(mapRefPoint.Y()));
    info("\tprojectedRefPoint " + std::to_string(projectedRefPoint.X()) + " " + std::to_string(projectedRefPoint.Y()));
    info("\tprojectedCrsDesc '" + projectedCrsDesc + "'");
    info("\tgeographicCrsDesc '" + geographicCrsDesc + "'");
#endif // DEBUG
}

void Linear::translate(const Coords& delta) {
    x -= delta.X();
    y -= delta.Y();
}

void Linear::rotate(double angle) { ///< angle in radians!!!
    Coords tmp(x * cos(angle) - y * sin(angle),
               x * sin(angle) + y * cos(angle));
    x = tmp.X();
    y = tmp.Y();
}

void Linear::scale(double scaleX, double scaleY) {
    x *= scaleX;
    y *= scaleY;
}

Coords&
CoordsTransform::projToMap(Coords& coords) {
    Linear linear(coords);
    Coords delta = projectedRefPoint;
    linear.translate(delta);
    linear.rotate(grivation * DEG_TO_RAD);
    linear.scale(mapScale, -mapScale);
    Coords mapDelta = mapRefPoint * -1;
    linear.translate(mapDelta);
    linear.scale(1000,1000);
    return coords = linear;
}

Coords&
CoordsTransform::geographicToMap(Coords& coords) {
    if (!isInited()) {
        throw Error("CoordsTransform not inited!");
    }
    coords *= DEG_TO_RAD;
    double x = coords.X();
    double y = coords.Y();
    if (pj_transform(geographic_crs, projected_crs, 1, 1, &x, &y, nullptr)) {
        throw Error("geographic to map transform failed");
    }
    coords = Coords(x,y);
    coords = projToMap(coords);
    coords = Coords(round(coords.X()), round(coords.Y()));
    return coords;
}

