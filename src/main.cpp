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
#include <fstream>
#include <typeinfo>
#include <map>

#include "yaml-cpp/yaml.h"

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
<<<<<<< HEAD:main.cpp
        int id = Main::rules.getSymbolId(osmNode.getTagMap(), ElemType::node);
=======
        auto& tagMap = osmNode.getTagMap();
        const Symbol symbol = Main::rules.groupList.getSymbol(tagMap, ElemType::node);
        //info("sym id %d",id);
>>>>>>> master:src/main.cpp
        Coords coords = osmNode.getCoords();
        coords = Main::transform.geographicToMap(coords);
        if (id != invalid_sym_id) {
<<<<<<< HEAD:main.cpp
            if (Main::rules.isText(id)) {
                const std::string text = osmNode.getName();
                if (!text.empty()) {
                    xmapTree.add(id, coords, text.c_str());
                }
            }
            else {
                xmapTree.add(id, coords);
=======
            xmapTree.add(id, tagMap, coords);
        }
        int textId = symbol.TextId();
        if (textId != invalid_sym_id) {
            const std::string text = osmNode.getName();
            if (!text.empty()) {
                xmapTree.add(textId, tagMap, coords, text.c_str());
>>>>>>> master:src/main.cpp
            }
        }
    }

    Coords addCoordsToWay(XmapWay& way, int id, OsmWay osmWay, bool reverse = false) {
        if (reverse) {
            osmWay.reverse();
        }
        Coords lastGeographicCoords;
        for (const auto& osmNode : osmWay) {
            int flags = 0;
            if (Main::rules.isDashPoint(osmNode.getTagMap(),id)) {
                flags = 32;
            }
            Coords coords = osmNode.getCoords();
            lastGeographicCoords = coords;
            coords = Main::transform.geographicToMap(coords);
            way.addCoord(coords,flags);
        }
        return lastGeographicCoords;
    }

    template< >
    void handle(OsmWay& osmWay, XmapTree& xmapTree) {
<<<<<<< HEAD:main.cpp
        int id = Main::rules.getSymbolId(osmWay.getTagMap(), ElemType::way);
        XmapWay way = xmapTree.add(id);
        addCoordsToWay(way,id,osmWay);
=======
        auto& tagMap = osmWay.getTagMap();
        const Symbol symbol = Main::rules.groupList.getSymbol(tagMap, ElemType::way);
        XmapWay way = xmapTree.add(symbol.Id(), tagMap);
        addCoordsToWay(way,symbol,osmWay);
>>>>>>> master:src/main.cpp
    }

    template< >
    void handle(OsmRelation& osmRelation, XmapTree& xmapTree) {
        if (!osmRelation.isMultipolygon()) {
            return;
        }
<<<<<<< HEAD:main.cpp
        int id = Main::rules.getSymbolId(osmRelation.getTagMap(), ElemType::area);
        XmapWay way = xmapTree.add(id);
=======
        auto& tagMap = osmRelation.getTagMap();
        const Symbol symbol = Main::rules.groupList.getSymbol(tagMap, ElemType::area);
        XmapWay way = xmapTree.add(symbol.Id(), tagMap);
>>>>>>> master:src/main.cpp
        OsmMemberList memberList = osmRelation;
        OsmWay& osmWay = memberList.front();
        Coords lastCoords = addCoordsToWay(way,id,osmWay);
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
                lastCoords = addCoordsToWay(way,id,*iterator,reverse);
                memberList.erase(iterator);
            }
            else {
                way.completeMultipolygonPart();
                OsmWay& osmWay = memberList.front();
                lastCoords = addCoordsToWay(way,id,osmWay);
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

void osmToXmap(XmlElement& inOsmRoot, const char * outXmapFilename, const char * xmapSymbolFilename, const Georeferencing& georef) {

    XmapTree xmapTree(xmapSymbolFilename);
    xmapTree.setGeoreferencing(georef);
 
    info("Converting nodes...");
    Main::handleOsmData<OsmNode>(inOsmRoot,xmapTree);
    info("Ok");
    info("Converting ways...");
    Main::handleOsmData<OsmWay>(inOsmRoot,xmapTree);
    info("Ok");

    Coords min = OsmNode::getMinCoords();
    min = Main::transform.geographicToMap(min);
    Coords max = OsmNode::getMaxCoords();
    max = Main::transform.geographicToMap(max);

    info("Converting relations...");
    Main::handleOsmData<OsmRelation>(inOsmRoot,xmapTree);
    info("Ok");

    for (const auto id : Main::rules.backgroundList) {
        xmapTree.add(id, min, max);
    }

    xmapTree.save(outXmapFilename);
}

void printUsage(const char* programName) {
    info("Usage:");
    info("   " + std::string(programName) + " [options]");
    info("   Options:");
    info("      -i filename - input OSM filename (in.osm as default);");
    info("      -o filename - output XMAP filename (out.xmap as default);");
    info("      -s filename - symbol set XMAP or OMAP filename (symbols.xmap as default)");
    info("                    (see /usr/share/openorienteering-mapper/symbol\\ sets/);");
    info("      -r filename - XML rules filename (rules.xml as default);");
    info("      --help, -h or help - this usage.");
}

//FIXME
void checkFileName(const char* fileName, const char* programName) {
    if (fileName == nullptr) {
        printUsage(programName);
        throw Error("empty filename");
    }
    if (fileName[0] == '-') {
        printUsage(programName);
        throw Error("bad filename");
    }
}

int main(int argc, const char* argv[]) 
{ 
    try {
        Timer timer;
        
        const char* symbolFileName = "./symbols.xmap";
        const char* rulesFileName    = "./rules.yaml";
        const char* inOsmFileName    = "./in.osm";
        const char* outXmapFileName  = "./out.xmap";

        if (argc > 1) {
            for (int i = 1; i < argc; ++i) {
                if (std::string(argv[i]) == "-i") {
                    inOsmFileName = argv[++i];
                    checkFileName(inOsmFileName,argv[0]);
                }
                else if (std::string(argv[i]) == "-o") {
                    outXmapFileName = argv[++i];
                    checkFileName(outXmapFileName,argv[0]);
                }
                else if (std::string(argv[i]) == "-s") {
                    symbolFileName = argv[++i];
                    checkFileName(symbolFileName,argv[0]);
                }
                else if (std::string(argv[i]) == "-r") {
                    rulesFileName = argv[++i];
                    checkFileName(rulesFileName,argv[0]);
                }
                else if (std::string(argv[i]) == "--help" ||
                         std::string(argv[i]) == "-h" ||
                         std::string(argv[i]) == "help") {
                    printUsage(argv[0]);
                    return 0;
                }
                else {
                    printUsage(argv[0]);
                    throw Error("Unknown option '" + std::string(argv[i]) + "'");
                }
            }
        }

        info("Using files:");
        info("\t* input OSM file       - " + std::string(inOsmFileName));
        info("\t* output XMAP file     - " + std::string(outXmapFileName));
        info("\t* symbol set XMAP file - " + std::string(symbolFileName));
        info("\t* rules file           - " + std::string(rulesFileName));

        XmlTree inXmapDoc(symbolFileName);
        XmlElement inXmapRoot = inXmapDoc.getChild("map");

        XmlTree inOsmDoc(inOsmFileName);
        XmlElement inOsmRoot = inOsmDoc.getChild("osm");

        const double min_supported_version = 0.5;
        const double max_supported_version = 0.6;

        double version = inOsmRoot.getAttribute<double>("version");
        if (version < min_supported_version || version > max_supported_version) {
            throw Error("OSM data version %.1f isn't supported" + std::to_string(version));
        }

        XmlElement bounds = inOsmRoot.getChild("bounds");
        if (bounds.isEmpty()) {
            throw Error("No bounds");
        }
        double x = bounds.getAttribute<double>("minlon") +
                   bounds.getAttribute<double>("maxlon");
        x /= 2;
        double y = bounds.getAttribute<double>("minlat") +
                   bounds.getAttribute<double>("maxlat");
        y /= 2;
        Coords geographic_ref_point(x,y);

        Georeferencing georef(inXmapRoot, geographic_ref_point);

        Main::transform = CoordsTransform(georef);
        SymbolIdByCodeMap symbolIds(inXmapRoot);
        Main::rules = Rules(rulesFileName,symbolIds);

        osmToXmap(inOsmRoot,outXmapFileName,symbolFileName,georef);

        // FIXME See https://github.com/jbeder/yaml-cpp/wiki/How-To-Parse-A-Document-%28Old-API%29
        // See https://github.com/liosha/osm2mp/blob/master/cfg/polish-mp/ways-roads-common-univ.yml 
        /*
        std::ifstream rulesFile("rules.yaml");
        YAML::Parser  parser(rulesFile);
        YAML::Node    doc;
        if (parser.GetNextDocument(doc)) {
            for (YAML::Iterator it = doc.begin(); it != doc.end(); ++it) {
                std::string code;
                it.first() >> code;
                info(code);
                const YAML::Node& tags = it.second();
                if (tags.Type() == YAML::NodeType::Map) {
                    for (YAML::Iterator it = tags.begin(); it != tags.end(); ++it) {
                        //info(YamlRules::type(it.first().Type()));
                        //info(YamlRules::type(it.second().Type()));
                        std::string key, value;
                        it.first() >> key;
                        it.second() >> value;
                        info(key + " = " + value);
                    }
                }
                else if (tags.Type() == YAML::NodeType::Scalar) {
                    std::string str;
                    tags >> str;
                    info("is " + str);
                }
                else {
                    info("undefined");
                }
            }
        }
        */

        info("\nExecution time: " + std::to_string(timer.getCurTime()) + " sec.");
    }
    /*
    catch (const char * msg) {
        std::cerr << "ERROR: " << msg << std::endl;
    }
    */
    catch (std::exception& error) {
        std::cerr << "ERROR: " << error.what() << std::endl;
        return 1;
    }
}

