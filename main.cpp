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
Coords addWayToMultipolygon(node_t * mp, node_t * way, bool reverse) {
    //printf("add way %ld %d\n",(long)way,reverse);
    static Coords previosCoords = {.x=0, .y=0};
    node_t * wayChld = roxml_get_chld(way,NULL,0);
    if (reverse) {
        wayChld = roxml_get_chld(way,NULL,roxml_get_chld_nb(way)-1);
    }
    int i = 0;
    while (wayChld != NULL) {
        char * chldName = roxml_get_name(wayChld,NULL,0);
        if (strcmp(chldName,"nd") == 0) {
            long pointId = xmlGetAttrValueL(wayChld,"ref");
            node_t * osmPointXmlElm = getSavedPoint(pointId);
            Coords coords = getPointCoords(osmPointXmlElm);
            if (i == 0 && coords.x == previosCoords.x && coords.y == previosCoords.y) {
                xmapRemoveFlags(mp);
            }
            else {
                int flags = 0;
                xmapAddCoordToObject(mp, &coords, flags);
            }
            previosCoords.x = coords.x;
            previosCoords.y = coords.y;
        }
        roxml_release(chldName);
        if (reverse) {
            wayChld = roxml_get_prev_sibling(wayChld);
        }
        else {
            wayChld = roxml_get_next_sibling(wayChld);
        }
        ++i;
    }
    xmapCompleteMultipolygonPart(mp);
    return previosCoords;
}

bool isMultipolygon(node_t * relation) {
    node_t * item = roxml_get_chld(relation,NULL,0);
    while (item != NULL) {
        char * itemName = roxml_get_name(item,NULL,0);
        if (strcmp(itemName,"tag")==0) {
            if (checkTagKeyAndValue(item,"type","multipolygon")) {
                roxml_release(itemName);
                return true;
            }
        }
        roxml_release(itemName);
        item = roxml_get_next_sibling(item);
    }
    return false;
}

struct _MpWayList {
    node_t * way;
    Coords firstCoords;
    Coords lastCoords;
    bool added;
    struct _MpWayList * next;
};

typedef struct _MpWayList MpWayList;

Coords mpGetWayFirstCoords(node_t * way) {
    node_t * firstNd = roxml_get_chld(way,"nd",0);
    long firstNdId = xmlGetAttrValueL(firstNd,"ref");
    return getPointById(firstNdId);
}

Coords mpGetWayLastCoords(node_t * way) {
    node_t * nextNd = roxml_get_chld(way,"nd",0);
    long lastNdId = 0;
    while (nextNd != NULL) {
        char * ndName = roxml_get_name(nextNd,NULL,0);
        if (strcmp(ndName,"nd")==0) {
            lastNdId = xmlGetAttrValueL(nextNd,"ref");
        }
        else {
            nextNd = NULL;
        }
        roxml_release(ndName);
        nextNd = roxml_get_next_sibling(nextNd);
    }
    return getPointById(lastNdId);
}

MpWayList * mpAddWayToList(MpWayList * mpWayList, node_t * way, Coords firstCoords, Coords lastCoords) {
    //printf("add way to list %ld %ld %ld %ld %ld\n",(long)way,(long)firstCoords.x,(long)firstCoords.y,(long)lastCoords.x,(long)lastCoords.y);
    MpWayList * newWayList = (MpWayList*)malloc(sizeof(MpWayList));
    newWayList->next = mpWayList;
    newWayList->way = way;
    newWayList->firstCoords = firstCoords;
    newWayList->lastCoords = lastCoords;
    return newWayList;
}

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

    template< >
    void handle(OsmWay& osmWay, XmapTree& xmapTree) {
        const Symbol symbol = Main::rules.groupList.getSymbol(osmWay.getTagMap(), ELEM_WAY);
        XmapWay way = xmapTree.add(symbol.Id());
        Tag ndSymbolTag = symbol.NdSymbolTag();
        for (OsmNodeList::iterator it = osmWay.begin();
             it != osmWay.end();
             ++it) {
            int flags = 0;
            if (!ndSymbolTag.empty()) {
                if(it->getTagMap().exist(ndSymbolTag)) {
                    flags = 32;
                }
            }
            Coords coords = it->getCoords();
            coords = Main::transform.geographicToMap(coords);
            way.addCoord(coords,flags);
        }
    }

    template< >
    void handle(OsmRelation& osmRelation, XmapTree& xmapTree) {
        const Symbol symbol = Main::rules.groupList.getSymbol(osmRelation.getTagMap(), ELEM_AREA);
        XmapWay way = xmapTree.add(symbol.Id());
        for (auto it = osmRelation.begin();
             it != osmRelation.end();
             ++it) {
            ////
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
