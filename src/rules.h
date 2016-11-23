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

#ifndef RULES_H_INCLUDED
#define RULES_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

#include <string>
#include <map>
#include <list>
#include "yaml-cpp/yaml.h"
#include "common.h"
#include "xmap.h"

namespace ElemType {
    const int node    = 1<<0;
    const int way     = 1<<1;
    const int area    = 1<<2;
}

typedef std::list<int> BackgroundList;

class IdAndTagMap
: public TagMap {
    int id;
public:
    IdAndTagMap(int _id) : id(_id) {};
    int getId() const { return id; };
};

typedef std::set<IdAndTagMap*> IdAndTagMapPSet;

class IdMap
: public std::map<std::string/*k=v*/, IdAndTagMapPSet> {
public:
    void debugPrint();
};

typedef std::map<int/*way id*/, TagMap> DashMap;

class Rules
: public TrueInit {
    void parseMap(const YAML::Node& yaml_map, int id);
public:
    //GroupList groupList;
    BackgroundList backgroundList;
    IdMap idMap;
    DashMap dashMap;
    Rules() {};
    Rules(const std::string&, SymbolIdByCodeMap&);
    int getSymbolId(const TagMap& checkedTags, int elemType);
    bool isDashPoint(const TagMap& checkedTags, int id);
    bool isText(int id);
};

#endif // RULES_H_INCLUDED

