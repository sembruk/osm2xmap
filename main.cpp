#include <iostream>
#include <typeinfo>
#include <map>

#include "timer.h"
#include "xml.h"
#include "xmap.h"
#include "osm.h"
#include "coordsTransform.h"
#include "rules.h"

#include "common.h"
//using namespace std;

namespace Main {
    CoordsTransform transform;
    Rules rules;

    template< typename T >
    void handle(T&, XmapTree&);

    template< >
    void handle(OsmNode& osmNode, XmapTree& xmapTree) {
        const Symbol symbol = Main::rules.groupList.getSymbol(osmNode.getTagMap(), ELEM_NODE);
        //info("sym id %d",id);
        Coords coords = osmNode.getCoords();
        coords = Main::transform.geographicToMap(coords);
        int id = symbol.Id();
        if (id != invalid_sym_id) {
            xmapTree.add(id, coords);
        }
        int textId = symbol.TextId();
        if (textId != invalid_sym_id) {
            const std::string text = osmNode.getName();
            if (!text.empty()) {
                xmapTree.add(textId, coords, text.c_str());
            }
        }
    }

    Coords addCoordsToWay(XmapWay& way, const Symbol& symbol, OsmWay osmWay, bool reverse = false) {
        if (reverse) {
            osmWay.reverse();
        }
        Tag dashSymbolTag = symbol.NdSymbolTag();
        Coords lastGeographicCoords;
        for (OsmNodeList::iterator it = osmWay.begin();
             it != osmWay.end();
             ++it) {
            int flags = 0;
            if (!dashSymbolTag.empty()) {
                if(it->getTagMap().exist(dashSymbolTag)) {
                    flags = 32;
                }
            }
            Coords coords = it->getCoords();
            lastGeographicCoords = coords;
            coords = Main::transform.geographicToMap(coords);
            way.addCoord(coords,flags);
        }
        return lastGeographicCoords;
    }

    template< >
    void handle(OsmWay& osmWay, XmapTree& xmapTree) {
        const Symbol symbol = Main::rules.groupList.getSymbol(osmWay.getTagMap(), ELEM_WAY);
        XmapWay way = xmapTree.add(symbol.Id());
        addCoordsToWay(way,symbol,osmWay);
    }

    template< >
    void handle(OsmRelation& osmRelation, XmapTree& xmapTree) {
        if (!osmRelation.isMultipolygon()) {
            return;
        }
        const Symbol symbol = Main::rules.groupList.getSymbol(osmRelation.getTagMap(), ELEM_AREA);
        XmapWay way = xmapTree.add(symbol.Id());
        OsmMemberList memberList = osmRelation;
        OsmWay& osmWay = memberList.front();
        Coords lastCoords = addCoordsToWay(way,symbol,osmWay);
        memberList.pop_front();
        ///TODO check member role
        while (!memberList.empty()) {
            bool found = false;
            bool reverse = false;
            OsmMemberList::iterator iterator;
            for (auto it = memberList.begin();
                 it != memberList.end();
                 ++it) {
                iterator = it;
                if (lastCoords == iterator->getFirstCoords()) {
                    found = true;
                    reverse = false;
                    break;
                }
                if (lastCoords == iterator->getLastCoords()) {
                    found = true;
                    reverse = true;
                    break;
                }
            }
            if (found) {
                lastCoords = addCoordsToWay(way,symbol,*iterator,reverse);
                memberList.erase(iterator);
            }
            else {
                way.completeMultipolygonPart();
                OsmWay& osmWay = memberList.front();
                lastCoords = addCoordsToWay(way,symbol,osmWay);
                memberList.pop_front();
            }
        }
    }

    template< typename T >
    void handleOsmData(XmlElement& osmRoot, XmapTree& xmapTree) {
        for ( XmlElement item = osmRoot.getChild();
              !item.isEmpty();
              ++item ) {
            if (item == T::name()) {
                T obj(item);
                handle(obj,xmapTree);
            }
        }
    }
}

void osmToXmap(const char * inOsmFilename, const char * outXmapFilename, const char * xmapTemplateFilename) {
    const double min_supported_version = 0.5;
    const double max_supported_version = 0.6;

    XmlTree inOsmDoc(inOsmFilename);
    XmlElement inOsmRoot = inOsmDoc.getChild("osm");

    double version = inOsmRoot.getAttribute<double>("version");
    if (version < min_supported_version || version > max_supported_version) {
        error("OSM data version %.1f isn't supported",version);
    }

    XmlElement bounds = inOsmRoot.getChild("bounds");
    if (!bounds.isEmpty()) {
        info("Have bounds");
    }
    XmapTree xmapTree(xmapTemplateFilename);
 
    Main::handleOsmData<OsmNode>(inOsmRoot,xmapTree);
    Main::handleOsmData<OsmWay>(inOsmRoot,xmapTree);

    Coords min = OsmNode::getMinCoords();
    min = Main::transform.geographicToMap(min);
    Coords max = OsmNode::getMaxCoords();
    max = Main::transform.geographicToMap(max);

    Main::handleOsmData<OsmRelation>(inOsmRoot,xmapTree);

    for (BackgroundList::iterator it = Main::rules.backgroundList.begin();
         it != Main::rules.backgroundList.end();
         ++it) {
        int id = *it;
        xmapTree.add(id, min, max);
    }

    xmapTree.save(outXmapFilename);
}

int main(int argc, const char* argv[]) 
{ 
    try {
        Timer timer;

        const char* templateFileName = "./template.xmap";
        const char* rulesFileName    = "./rules.xml";
        const char* inOsmFileName    = "./in.osm";
        const char* outXmapFileName  = "./out.xmap";

        if (argc > 1) {
        }
        else {
            info("Using default values:");
            info("\t* input OSM file     - %s",inOsmFileName);
            info("\t* output XMAP file   - %s",outXmapFileName);
            info("\t* template XMAP file - %s",templateFileName);
            info("\t* rules file         - %s",rulesFileName);
        }

        XmlTree inXmapDoc(templateFileName);
        XmlElement inXmapRoot = inXmapDoc.getChild("map");

        Georeferencing georef(inXmapRoot);

        Main::transform = CoordsTransform(georef);
        SymbolIdByCodeMap symbolIds(inXmapRoot);
        Main::rules = Rules(rulesFileName,symbolIds);

        osmToXmap(inOsmFileName,outXmapFileName,templateFileName);

        info("\nВремя выполнения: %.0f сек.",timer.getCurTime());
    }
    catch (const char * msg) {
        std::cerr << "ERROR: " << msg << std::endl;
    }
    std::cout << "Пока!" << std::endl;
    return 0; 
}
