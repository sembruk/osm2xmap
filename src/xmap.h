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

#ifndef XMAP_H_INCLUDED
#define XMAP_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

#include <string>
#include <map>
#include "xml.h"
#include "common.h"

const unsigned crs_desc_len = 64;

enum class SymType : int {
    undef = -1,
    point = 1,
    way   = 2,
    text  = 8,
};

class SymbolIdByCodeMap
: public std::map<std::string, std::pair<int, SymType> > {
public:
    SymbolIdByCodeMap(XmlElement& root);
    int get(std::string code) const;
    SymType getType(int id) const;
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
    void addCoordsToEnd(const Coords& coords, int flags);
    void addCoordsToBegin(const Coords& coords, int flags);
private:
    void addCoords(const Coords& coords, bool toBegin, int flags);
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
