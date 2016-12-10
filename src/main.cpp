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

#ifndef VERSION_STRING
#define VERSION_STRING "undefined"
#endif

namespace Main {
    CoordsTransform transform;
    Rules rules;

    template< typename T >
    void handle(T&, XmapTree&, bool);

    template< >
    void handle(OsmNode& osmNode, XmapTree& xmapTree, bool show_invalid_sym_id) {
        auto& tagMap = osmNode.getTagMap();
        int id = Main::rules.getSymbolId(tagMap, ElemType::node);
        if ( id == invalid_sym_id &&
             (!show_invalid_sym_id || tagMap.empty()) ) {
            return;
        }
        Coords coords = osmNode.getCoords();
        coords = Main::transform.geographicToMap(coords);
        if (Main::rules.isText(id)) {
            const std::string text = osmNode.getName();
            if (!text.empty()) {
                xmapTree.add(id, tagMap, coords, text.c_str());
            }
        }
        else {
            xmapTree.add(id, tagMap, coords);
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
    void handle(OsmWay& osmWay, XmapTree& xmapTree, bool show_invalid_sym_id) {
        auto& tagMap = osmWay.getTagMap();
        int id = Main::rules.getSymbolId(tagMap, ElemType::way);
        if ( id == invalid_sym_id &&
             (!show_invalid_sym_id || tagMap.empty()) ) {
            return;
        }
        XmapWay way = xmapTree.add(id, tagMap);
        addCoordsToWay(way,id,osmWay);
    }

    template< >
    void handle(OsmRelation& osmRelation, XmapTree& xmapTree, bool show_invalid_sym_id) {
        if (!osmRelation.isMultipolygon()) {
            return;
        }
        auto& tagMap = osmRelation.getTagMap();
        int id = Main::rules.getSymbolId(tagMap, ElemType::area);
        if (id == invalid_sym_id && !show_invalid_sym_id) {
            return;
        }
        XmapWay way = xmapTree.add(id, tagMap);
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
    void handleOsmData(XmlElement& osmRoot, XmapTree& xmapTree, bool show_invalid_sym_id = true) {
        for ( XmlElement item = osmRoot.getChild();
              !item.isEmpty();
              ++item ) {
            if (item == T::name()) {
                T obj(item);
                handle(obj, xmapTree, show_invalid_sym_id);
            }
        }
    }
}

void osmToXmap(XmlElement& inOsmRoot, const char * outXmapFilename, const char * xmapSymbolFilename, const Georeferencing& georef, bool show_invalid_sym_id) {

    XmapTree xmapTree(xmapSymbolFilename);
    xmapTree.setGeoreferencing(georef);
 
    info("Converting nodes...");
    Main::handleOsmData<OsmNode>(inOsmRoot,xmapTree,show_invalid_sym_id);
    info("done");
    info("Converting ways...");
    Main::handleOsmData<OsmWay>(inOsmRoot,xmapTree,show_invalid_sym_id);
    info("done");

    OsmBounds bounds(inOsmRoot);
    Coords left_bottom = bounds.getMin();
    left_bottom = Main::transform.geographicToMap(left_bottom);
    Coords right_top = bounds.getMax();
    right_top = Main::transform.geographicToMap(right_top);

    Coords min(std::min(left_bottom.X(), right_top.X()),
               std::min(left_bottom.Y(), right_top.Y()));
    Coords max(std::max(left_bottom.X(), right_top.X()),
               std::max(left_bottom.Y(), right_top.Y()));

    info("Converting relations...");
    Main::handleOsmData<OsmRelation>(inOsmRoot,xmapTree,show_invalid_sym_id);
    info("done");

    for (const auto id : Main::rules.backgroundList) {
        xmapTree.add(id, min, max);
    }

    xmapTree.save(outXmapFilename);
}

void printVersion() {
    info("Version "+std::string(VERSION_STRING));
}

#ifdef SHAREDIR
const std::string defaultSymbolFileName   = SHAREDIR"/ISOM_15000.xmap";
const std::string defaultRulesFileName    = SHAREDIR"/ISOM_rules.yaml";
#else
const std::string defaultSymbolFileName   = "ISOM_15000.xmap";
const std::string defaultRulesFileName    = "ISOM_rules.yaml";
#endif
const std::string defaultInOsmFileName    = "in.osm";
const std::string defaultOutXmapFileName  = "out.xmap";

void printUsage(const char* programName) {
    info("Usage:");
    info("   " + std::string(programName) + " [options] [file]");
    info("   file - input OSM filename ('"+defaultInOsmFileName+"' as default);");
    info("   Options:");
    info("      -o filename        output XMAP filename ('"+defaultOutXmapFileName+"' as default);");
    info("      -s filename        symbol set XMAP or OMAP filename ('"+defaultSymbolFileName+"' as default)");
    info("                         (see /usr/share/openorienteering-mapper/symbol\\ sets/);");
    info("      -r filename        YAML rules filename ('"+defaultRulesFileName+"' as default);");
    info("      -n, --no-empty-objects");
    info("                         don`t convert objects without symbol;");
    info("      -V, --version      print software version;");
    info("      -h, --help         this usage.");
}

//FIXME
void checkFileName(const char* fileName, const char* programName) {
    if (fileName == nullptr) {
        printUsage(programName);
        throw Error("empty filename");
    }
    if (fileName[0] == '-') {
        printUsage(programName);
        throw Error("bad filename '"+std::string(fileName)+"'");
    }
}

int main(int argc, const char* argv[]) 
{ 
    try {
        Timer timer;
        
        const char* symbolFileName   = defaultSymbolFileName.c_str();
        const char* rulesFileName    = defaultRulesFileName.c_str();
        const char* inOsmFileName    = defaultInOsmFileName.c_str();
        const char* outXmapFileName  = defaultOutXmapFileName.c_str();

        bool show_invalid_sym_id = true;

        if (argc > 1) {
            for (int i = 1; i < argc; ++i) {
                std::string option(argv[i]);
                if (option == "-o") {
                    outXmapFileName = argv[++i];
                    checkFileName(outXmapFileName,argv[0]);
                }
                else if (option == "-s") {
                    symbolFileName = argv[++i];
                    checkFileName(symbolFileName,argv[0]);
                }
                else if (option == "-r") {
                    rulesFileName = argv[++i];
                    checkFileName(rulesFileName,argv[0]);
                }
                else if (option == "-n" ||
                         option == "--no-empty-objects") {
                    show_invalid_sym_id = false;
                }
                else if (option == "-V" ||
                         option == "--version") {
                    printVersion();
                    return 0;
                }
                else if (option == "--help" ||
                         option == "-h") {
                    printUsage(argv[0]);
                    return 0;
                }
                else if (option[0] == '-') {
                    throw Error("Invalid option '" + option + "'");
                }
                else {
                    inOsmFileName = argv[i];
                    checkFileName(inOsmFileName,argv[0]);
                }
            }
        }

        printVersion();
        info("Using files:");
        info("   * input OSM file       - " + std::string(inOsmFileName));
        info("   * output XMAP file     - " + std::string(outXmapFileName));
        info("   * symbol set XMAP file - " + std::string(symbolFileName));
        info("   * rules file           - " + std::string(rulesFileName));

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

        OsmBounds bounds(inOsmRoot);
        double lon = bounds.getMin().X() +
                     bounds.getMax().X();
        lon /= 2;
        double lat = bounds.getMin().Y() +
                     bounds.getMax().Y();
        lat /= 2;
        Coords geographic_ref_point(lon,lat);

        Georeferencing georef(inXmapRoot, geographic_ref_point);

        Main::transform = CoordsTransform(georef);
        SymbolIdByCodeMap symbolIds(inXmapRoot);
        Main::rules = Rules(rulesFileName,symbolIds);

        osmToXmap(inOsmRoot, outXmapFileName, symbolFileName, georef, show_invalid_sym_id);

        // FIXME See https://github.com/jbeder/yaml-cpp/wiki/How-To-Parse-A-Document-%28Old-API%29
        // See https://github.com/liosha/osm2mp/blob/master/cfg/polish-mp/ways-roads-common-univ.yml 

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

