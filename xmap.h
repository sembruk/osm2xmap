#ifndef XMAP_H_INCLUDED
#define XMAP_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

#include <string>
#include <map>
#include "xml.h"
//#include "rules.h"
#include "common.h"

const unsigned crs_desc_len = 64;

/*
class CoordsTransform;

class Georeferencing {
protected:
    Coords mapRefPoint;
    Coords projectedRefPoint;
    double mapScale;
    double grivation; ///< deg
    std::string geographicCrsDesc;
    std::string projectedCrsDesc;

    friend class CoordsTransform;
public:
    Georeferencing(XmlElement& root);
    Georeferencing() {};
};
*/

enum class SymType : int {
    undef = -1,
    point = 1,
    way   = 2,
    text  = 8,
};

class SymbolIdByCodeMap
: public std::map<std::string, int> {
public:
    SymbolIdByCodeMap(XmlElement& root);
    int get(std::string code) const;
};

class XmapTree;

class XmapObject {
protected:
    XmlElement objectElement;
    XmlElement coordsElement;
    XmlElement lastCoordElement;

    Coords first;
    Coords last;
 
    XmapObject(XmapTree* xmapTree, int id, const TagMap& tagMap);
public:
    void addCoord(const Coords& coords, int flags);
};

class XmapPoint
: public XmapObject {
public:
    XmapPoint(XmapTree* xmapTree, int id, const TagMap& tagMap, Coords& coords);
};

class XmapWay
: public XmapObject {
public:
    XmapWay(XmapTree* xmapTree, int id, const TagMap& tagMap);
    ~XmapWay();

    void completeMultipolygonPart();
    void removeFlags();
};

class XmapRectagngle
: public XmapWay {
public:
    XmapRectagngle(XmapTree* xmapTree, int id, Coords& min, Coords& max);
};

class XmapText
: public XmapPoint {
public:
    XmapText(XmapTree* xmapTree, int id, const TagMap& tagMap, Coords& coords, const char * text);
};

class Georeferencing;

class XmapTree {
    XmlTree tree;
    XmlElement map;
    XmlElement objects;
    int objectsCount;
    friend class XmapObject;
public:
    XmapTree(const char * templateFilename);
    void save(const char * outXmapFilename);
    void setGeoreferencing(const Georeferencing& georef);
    XmapPoint      add(int id, const TagMap& tagMap, Coords& coords)           { return XmapPoint(this,id,tagMap,coords); };
    XmapWay        add(int id, const TagMap& tagMap)                           { return XmapWay(this,id,tagMap); };
    XmapRectagngle add(int id, Coords& min, Coords& max)                       { return XmapRectagngle(this,id,min,max); };
    XmapText       add(int id, const TagMap& tagMap, Coords& coords, const char * text) { return XmapText(this,id,tagMap,coords,text); };
};

#endif // XMAP_H_INCLUD;
