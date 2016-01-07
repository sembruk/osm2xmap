#include <iostream>
#include "common.h"
#include "xml.h"
#include "xmap.h"

Georeferencing::Georeferencing(XmlElement& root) {

    XmlElement georeferencingNode = root.getChild("georeferencing");
    scaleFactor = georeferencingNode.getAttribute<double>("scale");
    grivation = georeferencingNode.getAttribute<double>("grivation");

    XmlElement mapRefPointNode = georeferencingNode.getChild("ref_point");
    mapRefPoint = Coords(mapRefPointNode.getAttribute<double>("x"),
                         mapRefPointNode.getAttribute<double>("y"));

    XmlElement projRefPointNode = georeferencingNode.getChild("projected_crs"); 
    XmlElement projRefPointChldNode = projRefPointNode.getChild("ref_point");
    projectedRefPoint = Coords(projRefPointChldNode.getAttribute<double>("x"),
                               projRefPointChldNode.getAttribute<double>("y"));

    XmlElement projRefPointSpecNode = projRefPointNode.getChild("spec");
    projectedCrsDesc = projRefPointSpecNode.getContent();

    XmlElement geographicCrsNode = georeferencingNode.getChild("geographic_crs");
    XmlElement geographicSpecNode = geographicCrsNode.getChild("spec");
    geographicCrsDesc = geographicSpecNode.getContent();

#ifdef DEBUG
    info("Loaded georeferencing:");
    info("\tscaleFactor %d",scaleFactor);
    info("\tgrivation %f",grivation);
    info("\tmapRefPoint %f %f",mapRefPoint.X(),mapRefPoint.Y());
    info("\tprojectedRefPoint %f %f",projectedRefPoint.X(),projectedRefPoint.Y());
    info("\tprojectedCrsDesc '%s'",projectedCrsDesc.c_str());
    info("\tgeographicCrsDesc '%s'",geographicCrsDesc.c_str());
#endif // DEBUG
}

SymbolIdByCodeMap::SymbolIdByCodeMap(XmlElement& root) {
    XmlElement symbolsNode = root.getChild("symbols");
    //int nSymbols = symbolsNode.getAttribute<int>("count");

    for ( XmlElement item = symbolsNode.getChild();
          !item.isEmpty();
          ++item ) {
        if (item == "symbol") {
            int id = item.getAttribute<int>("id");
            std::string code = item.getAttribute<std::string>("code");
            //int type = item.getAttribute<int>("type");
            //std::string name = item.getAttribute<std::string>("name");
            //info("%d %s %s %d",id,code.c_str(),name.c_str(),type);
            (*this)[code] = id;
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
        throw Error("Symbol with code %s didn't find", code.c_str());
    }
    return it->second;
}

XmapTree::XmapTree(const char * templateFilename)
: tree(templateFilename) {
    XmlElement map = tree.getChild("map");
    XmlElement parts = map.getChild("parts");
    XmlElement part = parts.getChild("part");
    objects = part.getChild("objects");
    objectsCount = objects.getAttribute<int>("count");
}

void 
XmapTree::save(const char * outXmapFilename) { ///< void xmapSaveTreeInFile();
    objects.addAttribute("count",objectsCount);
    tree.saveInFile(outXmapFilename);
}


XmapObject::XmapObject(XmapTree* xmapTree, int id) {
    if (xmapTree == nullptr) {
        throw Error("xmap tree not inited");
    }
    objectElement = XmlElement(xmapTree->objects.addChild("object"));
    coordsElement = XmlElement(objectElement.addChild("coords"));
    objectElement.addAttribute("symbol",id);
    xmapTree->objectsCount++;
}

void
XmapObject::addCoord(const Coords& coords, int flags=0) {
    if (coordsElement.getChildNumber() == 0) {
        first = coords;
    }
    XmlElement coord(coordsElement.addChild("coord"));
    coord.addAttribute("x", coords.X());
    coord.addAttribute("y", coords.Y());
    if (flags != 0) {
        coord.addAttribute("flags", flags);
    }
    lastCoordElement = coord;
    last = coords;
}

XmapPoint::XmapPoint(XmapTree* xmapTree, int id, Coords& coords)
: XmapObject(xmapTree,id) {
    objectElement.addAttribute("type",0);
    coordsElement.addAttribute("count",1);
    addCoord(coords);
}

XmapWay::XmapWay(XmapTree* xmapTree, int id) : XmapObject(xmapTree,id) {
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

XmapText::XmapText(XmapTree* xmapTree, int id, Coords& coords, const char * text)
: XmapPoint(xmapTree, id, coords) { ///< xmapAddText()
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

