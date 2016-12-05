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

#include <iostream>
#include "common.h"
#include "xml.h"
#include "coordsTransform.h"
#include "xmap.h"

SymbolIdByCodeMap::SymbolIdByCodeMap(XmlElement& root) {
    XmlElement symbolsNode = root.getChild("symbols");
    if (symbolsNode.isEmpty()) {
        XmlElement barrierNode = root.getChild("barrier");
        symbolsNode = barrierNode.getChild("symbols");
    }
    //int nSymbols = symbolsNode.getAttribute<int>("count");

    for ( XmlElement item = symbolsNode.getChild();
          !item.isEmpty();
          ++item ) {
        if (item == "symbol") {
            int id = item.getAttribute<int>("id");
            std::string code = item.getAttribute<std::string>("code");
            int type = item.getAttribute<int>("type");
            //std::string name = item.getAttribute<std::string>("name");
            //info("%d %s %s %d",id,code.c_str(),name.c_str(),type);
            (*this)[code] = std::pair<int, SymType>(id, (SymType)type);
        };
    }
}

int
SymbolIdByCodeMap::get(std::string code) const {
    if (code.empty()) {
        return invalid_sym_id;
    }
    auto it = find(code);
    if (it == end()) {
        warning("Symbol with code " + code + " didn't find in the symbol set file");
        return invalid_sym_id;
    }
    return it->second.first;
}
SymType
SymbolIdByCodeMap::getType(int id) const {
    if (id != invalid_sym_id) {
        for (auto it = begin(); it != end(); ++it) {
            if (it->second.first == id) {
                return it->second.second;
            }
        }
    }
    return SymType::undef;
}

XmapTree::XmapTree(const char * templateFilename)
: tree(templateFilename) {
    map = tree.getChild("map");
    XmlElement parts = map.getChild("parts");
    if (parts.isEmpty()) {
        for ( XmlElement item = map.getChild();
              !item.isEmpty();
              ++item ) {
            if (item == "barrier") {
                parts = item.getChild("parts");
                if (!parts.isEmpty()) {
                    break;
                }
            }
        }
    }
    XmlElement part = parts.getChild("part");
    part.removeChild("objects");
    objects = part.addChild("objects");
    //objectsCount = objects.getAttribute<int>("count");
    objectsCount = 0;
}

void 
XmapTree::save(const char * outXmapFilename) { ///< void xmapSaveTreeInFile();
    objects.addAttribute<int>("count",objectsCount);
    info("Saving '"+std::string(outXmapFilename)+"'");
    tree.saveInFile(outXmapFilename,false);
}

void
XmapTree::setGeoreferencing(const Georeferencing& georef) {
    XmlElement georef_elem = map.getChild("georeferencing");
    georef_elem.addAttribute<int>("scale",int(1000.0/georef.mapScale));
    georef_elem.addAttribute("declination",georef.declination);
    georef_elem.addAttribute("grivation",georef.grivation);
    georef_elem.removeChild("ref_point");
    XmlElement ref_point = georef_elem.addChild("ref_point");
    ref_point.addAttribute<int>("x",georef.mapRefPoint.X());
    ref_point.addAttribute<int>("y",georef.mapRefPoint.Y());

    georef_elem.removeChild("projected_crs");
    XmlElement proj_crs_elem = georef_elem.addChild("projected_crs");
    //proj_crs_elem.addAttribute("id","EPSG");
    proj_crs_elem.addAttribute("id","UTM");

    XmlElement proj_ref_point = proj_crs_elem.addChild("ref_point");
    proj_ref_point.addAttribute("x",georef.projectedRefPoint.X());
    proj_ref_point.addAttribute("y",georef.projectedRefPoint.Y());

    XmlElement proj_spec_elem = proj_crs_elem.addChild("spec");
    proj_spec_elem.addAttribute("language","PROJ.4");
    proj_spec_elem.addContent(georef.projectedCrsDesc.c_str());

    XmlElement parameter_elem = proj_crs_elem.addChild("parameter");
    parameter_elem.addContent(georef.parameter.c_str());

    georef_elem.removeChild("geographic_crs");
    XmlElement geographic_crs_elem = georef_elem.addChild("geographic_crs");
    geographic_crs_elem.addAttribute("id","Geographic coordinates");

    XmlElement geographic_ref_point = geographic_crs_elem.addChild("ref_point_deg");
    geographic_ref_point.addAttribute("lat",georef.geographicRefPoint.Y());
    geographic_ref_point.addAttribute("lon",georef.geographicRefPoint.X());


    XmlElement geographic_spec_elem = geographic_crs_elem.addChild("spec");
    geographic_spec_elem.addAttribute("language","PROJ.4");
    geographic_spec_elem.addContent(georef.geographicCrsDesc.c_str());
}

XmapObject::XmapObject(XmapTree* xmapTree, int id, const TagMap& tagMap) {
    if (xmapTree == nullptr) {
        throw Error("xmap tree not inited");
    }
    objectElement = XmlElement(xmapTree->objects.addChild("object"));
    coordsElement = XmlElement(objectElement.addChild("coords"));
    objectElement.addAttribute("symbol",id);
    xmapTree->objectsCount++;

    XmlElement tagsElement(objectElement.addChild("tags"));
    for (auto& tag : tagMap) {
        XmlElement tElement(tagsElement.addChild("t"));
        tElement.addAttribute("k",tag.second->getKey());
        tElement.addContent(tag.second->getValue().c_str());
    }
}

void
XmapObject::addCoord(const Coords& coords, int flags=0) {
    if (coordsElement.getChildNumber() == 0) {
        first = coords;
    }
    XmlElement coord(coordsElement.addChild("coord"));
    coord.addAttribute<int>("x", coords.X());
    coord.addAttribute<int>("y", coords.Y());
    if (flags != 0) {
        coord.addAttribute("flags", flags);
    }
    lastCoordElement = coord;
    last = coords;
}

XmapPoint::XmapPoint(XmapTree* xmapTree, int id, const TagMap& tagMap, Coords& coords)
: XmapObject(xmapTree,id,tagMap) {
    objectElement.addAttribute("type",0);
    coordsElement.addAttribute("count",1);
    addCoord(coords);
}

XmapWay::XmapWay(XmapTree* xmapTree, int id, const TagMap& tagMap = {})
: XmapObject(xmapTree,id,tagMap) {
    objectElement.addAttribute("type",1);
    XmlElement patternElement(objectElement.addChild("pattern"));
    patternElement.addAttribute("rotation",0);
}

XmapWay::~XmapWay() { ///< xmapCompleteWay()
    int count = coordsElement.getChildNumber();
    coordsElement.addAttribute("count",count);
    if (first == last) {
        lastCoordElement.addAttribute("flags",50);
    }
}

void
XmapWay::completeMultipolygonPart() {
    //lastCoordElement.addAttribute("flags",16);
    lastCoordElement.addAttribute("flags",18);
}

void
XmapWay::removeFlags() { ///< xmapRemoveFlags()
    lastCoordElement.removeAttribute("flags");
}

XmapRectagngle::XmapRectagngle(XmapTree* xmapTree, int id, Coords& min, Coords& max)
: XmapWay(xmapTree, id) { ///< xmapAddRectangle()
    Coords left (max.X(), min.Y());
    Coords right(min.X(), max.Y());
    addCoord(min);
    addCoord(left);
    addCoord(max);
    addCoord(right);
    addCoord(min); ///< close way
}

XmapText::XmapText(XmapTree* xmapTree, int id, const TagMap& tagMap, Coords& coords, const char * text)
: XmapPoint(xmapTree, id, tagMap, coords) { ///< xmapAddText()
    if (text == nullptr) {
        return;
    }
    objectElement.addAttribute("type",4);
    objectElement.addAttribute("rotation",0);
    objectElement.addAttribute("h_align",1);
    objectElement.addAttribute("v_align",2);
    XmlElement textElement(objectElement.addChild("text"));
    textElement.addContent(text);
}

