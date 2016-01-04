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

/*
bool MpAllWaysAdded(MpWayList * mpWayList) {
    if (mpWayList) {
        if (mpWayList->added) {
            return MpAllWaysAdded(mpWayList->next);
        }
        else {
            return false;
        }
    }
    return true;
}

Coords mpAddWayToMp(node_t * mp, MpWayList * mpWayList, Coords lastAddedCoords) {
    if (mpWayList != NULL) {
        if (coordsEqual(&lastAddedCoords,&defCoords)) {
            if (mpWayList->added) {
                return mpAddWayToMp(mp, mpWayList->next, lastAddedCoords);
            }
            else {
                mpWayList->added = true;
                return addWayToMultipolygon(mp,mpWayList->way,false);
            }
        }
        if (mpWayList->added) {
            return mpAddWayToMp(mp, mpWayList->next, lastAddedCoords);
        }
        if (coordsEqual(&mpWayList->firstCoords,&lastAddedCoords)) {
            mpWayList->added = true;
            return addWayToMultipolygon(mp,mpWayList->way,false);
        }
        else if (coordsEqual(&mpWayList->lastCoords,&lastAddedCoords)) {
            mpWayList->added = true;
            return addWayToMultipolygon(mp,mpWayList->way,true);
        }
        return mpAddWayToMp(mp, mpWayList->next, lastAddedCoords);
    }
    return defCoords;
}

void handleRelation(node_t * relation) {
    if (isMultipolygon(relation)) {
        int id = rulesGetSymId(relation,ELEM_POLYGON);
        if (id == INVALID_SYM_ID) {
            return;
        }
        node_t * mp = xmapAddWay();
        xmapSetObjectSymbolId(mp,id);

        MpWayList * mpWayList = NULL;
        node_t * way = NULL;
        node_t * item = roxml_get_chld(relation,NULL,0);
        while (item != NULL) {
            char * itemName = roxml_get_name(item,NULL,0);
            if (strcmp(itemName,"member")==0) {
                char * type = xmlGetAttrValueS(item,"type");
                if (strcmp(type,"way")==0) {
                    long memberId = xmlGetAttrValueL(item,"ref");
                    way = getSavedWay(memberId);
                    if (way != NULL) {
                        Coords first = mpGetWayFirstCoords(way);
                        Coords last = mpGetWayLastCoords(way);
                        if (coordsEqual(&first,&last)) {
                            addWayToMultipolygon(mp,way,false);
                        }
                        else {
                            mpWayList = mpAddWayToList(mpWayList,way,first,last);
                        }
                    }
                }
                roxml_release(type);
            }
            roxml_release(itemName);
            item = roxml_get_next_sibling(item);
        }

        Coords lastAddedCoords = mpAddWayToMp(mp, mpWayList, defCoords);
        while (lastAddedCoords.x != 0 && lastAddedCoords.y != 0) {
            lastAddedCoords = mpAddWayToMp(mp, mpWayList, lastAddedCoords);
            if (coordsEqual(&lastAddedCoords,&defCoords)) {
                lastAddedCoords = mpAddWayToMp(mp, mpWayList, lastAddedCoords);
            }
            //printf("%ld %ld\n",(long)lastAddedCoords.x,(long)lastAddedCoords.y);
        }
        if (! MpAllWaysAdded(mpWayList) ) {
            ERROR("some ways not added");
        }

        xmapCompleteWay(mp);
    }
}
*/

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
            for (auto it = memberList.begin();
                 it != memberList.end();
                 ++it) {
                OsmWay& osmWay = *(it);
                if (lastCoords == osmWay.getFirstCoords()) {
                    addCoordsToWay(way,symbol,osmWay,false);
                    lastCoords = osmWay.getLastCoords();
                    memberList.erase(it);
                    found = true;
                    break;
                }
                if (lastCoords == osmWay.getLastCoords()) {
                    addCoordsToWay(way,symbol,osmWay,true);
                    lastCoords = osmWay.getFirstCoords();
                    memberList.erase(it);
                    found = true;
                    break;
                }
            }
            if (!found) {
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
    //info("Min (%f,%f)",min.X(),min.Y());
    min = Main::transform.geographicToMap(min);
    Coords max = OsmNode::getMaxCoords();
    //info("Max (%f,%f)",max.X(),max.Y());
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

int main() 
{ 
    try {
        Timer timer;

        XmlTree inXmapDoc("template.xmap");
        XmlElement inXmapRoot = inXmapDoc.getChild("map");

        Georeferencing georef(inXmapRoot);

        Main::transform = CoordsTransform(georef);
        SymbolIdByCodeMap symbolIds(inXmapRoot);
        Main::rules = Rules("rules.xml",symbolIds);

        osmToXmap("in.osm","out.xmap","template.xmap");

        info("\nВремя выполнения: %.0f сек.",timer.getCurTime());
    }
    catch (const char * msg) {
        std::cerr << "ERROR: " << msg << std::endl;
    }
    std::cout << "Пока!" << std::endl;
    return 0; 
}
