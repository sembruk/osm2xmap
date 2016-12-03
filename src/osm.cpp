/*
 *    Copyright 2016 Semyon Yakimov
 *
 *    This file is part of Osm2xmap.
 *
 *    Osm2xmap is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Osm2xmap is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Osm2xmap.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "osm.h"

OsmBounds::OsmBounds(const XmlElement& osmElement) {
    XmlElement bounds = osmElement.getChild("bounds");
    if (bounds.isEmpty()) {
        throw Error("No bounds in OSM file");
    }
    min = Coords( bounds.getAttribute<double>("minlon"),
                  bounds.getAttribute<double>("minlat") );
    max = Coords( bounds.getAttribute<double>("maxlon"),
                  bounds.getAttribute<double>("maxlat") );
}

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
            tagMap.insert(Tag(k,v));
        }
    }
}

const std::string
OsmObject::getName() const {
    TagMap::const_iterator it = tagMap.find("name");
    if (it != tagMap.end()) {
        return it->second->getValue();
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
                warning("Node " + std::to_string(nodeId) + " didn't find");
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
        role = Role::inner;
    }
    else if (_role == "outer") {
        role = Role::outer;
    }
    else {
        Error("no role");
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
                //warning("Way " + std::to_string(memberId) + " didn't find"); ///< it's normal
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

