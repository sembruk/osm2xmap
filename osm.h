#ifndef OSM_H_INCLUDED
#define OSM_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

#include "rules.h"

class OsmObject {
protected:
    TagMap tagMap;
public:
    OsmObject() {};
    OsmObject(const XmlElement& osmElement);
    const TagMap& getTagMap() const { return tagMap; };
    const std::string getName() const;
};

class OsmNode
: public OsmObject {
    Coords coords;
    static Coords maxCoords;
    static Coords minCoords;
public:
    static const char * name() { return "node"; };
    OsmNode() {};
    OsmNode(XmlElement& _osmElement);
    Coords getCoords() const { return coords; };
    static const Coords& getMaxCoords() { return maxCoords; };
    static const Coords& getMinCoords() { return minCoords; };
};

typedef std::list<OsmNode> OsmNodeList;

class OsmWay
: public OsmObject, public OsmNodeList {
    //OsmNodeList nodeList;
	Coords firstCoords;
	Coords lastCoords;
public:
    static const char * name() { return "way"; };
    OsmWay() {};
    OsmWay(XmlElement& _osmElement);
    //OsmNodeList::iterator begin() { return nodeList.begin(); };
    //OsmNodeList::iterator end() { return nodeList.end(); };
    const Coords& getFirstCoords() const { return firstCoords; };
    const Coords& getLastCoords() const { return lastCoords; };
};

enum Role {
    OUTER = 0,
    INNER = 1,
};

class OsmMultipolygonMember
: public OsmWay {
    Role role;
public:
    OsmMultipolygonMember(OsmWay& osmWay, std::string& _role);
};

typedef std::list<OsmMultipolygonMember> OsmMemberList;

class OsmRelation
: public OsmObject, public OsmMemberList {
public:
    bool isMultipolygon() const;
    static const char * name() { return "relation"; };
    OsmRelation(XmlElement& osmElement);
};


namespace Osm {
    typedef std::map<long, OsmNode> NodeMap;
    typedef std::map<long, OsmWay>  WayMap;
    };



#endif // OSM_H_INCLUDED

