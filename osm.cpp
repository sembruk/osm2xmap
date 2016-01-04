#include "osm.h"

namespace Osm {
    NodeMap nodeMap;
    WayMap  wayMap;
};

OsmObject::OsmObject(const XmlElement& osmElement) {
    for ( XmlElement item = osmElement.getChild();
          !item.isEmpty();
          ++item ) {
        if (item == "tag") {
            std::string k = item.getAttribute<std::string>("k");
            std::string v = item.getAttribute<std::string>("v");
            Tag tag(k,v);
            tagMap.insert(tag);
        }
    }
}

const std::string
OsmObject::getName() const {
    TagMap::const_iterator it = tagMap.find("name");
    if (it != tagMap.end()) {
        return it->second.getValue();
    }
    return std::string();
}

const Coords invalidGeographicalCoords(-360,-360);
Coords OsmNode::maxCoords(invalidGeographicalCoords);
Coords OsmNode::minCoords(invalidGeographicalCoords);

OsmNode::OsmNode(XmlElement& osmElement)
: OsmObject(osmElement) { ///< getPointCoords()
    coords = Coords(osmElement.getAttribute<double>("lon"),
                    osmElement.getAttribute<double>("lat"));
    //coords = transform.geographicToMap(coords);
    if (maxCoords == invalidGeographicalCoords) {
        maxCoords = coords;
    }
    if (minCoords == invalidGeographicalCoords) {
        minCoords = coords;
    }
    maxCoords = Coords(maxX(maxCoords, coords),
                       maxY(maxCoords, coords));
    minCoords = Coords(minX(minCoords, coords),
                       minY(minCoords, coords));

    Osm::nodeMap[osmElement.getAttribute<long>("id")] = (*this);
}

OsmWay::OsmWay(XmlElement& osmElement)
: OsmObject(osmElement) {
	bool first = true;
    for ( XmlElement item = osmElement.getChild();
          !item.isEmpty();
          ++item ) {
        if (item == "nd") {
            long nodeId = item.getAttribute<long>("ref");
            auto it = Osm::nodeMap.find(nodeId);
            if (it == Osm::nodeMap.end()) {
                warning("Node %ld didn't find",nodeId);
            }
            else {
				OsmNode node = it->second;
                push_back(node);
				if (first) {
					firstCoords = node.getCoords();
					first = false;
				}
				lastCoords = node.getCoords();
            }
        }
    }

    Osm::wayMap[osmElement.getAttribute<long>("id")] = (*this);
}

OsmMultipolygonMember::OsmMultipolygonMember(OsmWay& osmWay, std::string& _role)
: OsmWay(osmWay) {
    if (_role == "inner") {
        role = INNER;
    }
    else if (_role == "outer") {
        role = OUTER;
    }
    else {
        error("no role");
    }
}

OsmRelation::OsmRelation(XmlElement& osmElement)
: OsmObject(osmElement) {
    if (!isMultipolygon()) {
        return;
    }
    for ( XmlElement item = osmElement.getChild();
          !item.isEmpty();
          ++item ) {
        if (item == "member" && item.getAttribute<std::string>("type") == "way") {
            long memberId = item.getAttribute<long>("ref");
            Osm::WayMap::iterator it = Osm::wayMap.find(memberId);
            if (it == Osm::wayMap.end()) {
                warning("Way %ld didn't find",memberId);
            }
            else {
                std::string role = item.getAttribute<std::string>("role");
                push_back(OsmMultipolygonMember(it->second,role));
            }
        }
    }
}

bool 
OsmRelation::isMultipolygon() const {
    return tagMap.exist(Tag("type","multipolygon"));
}

