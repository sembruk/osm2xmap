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
#include "common.h"
#include "xmap.h"

namespace ElemType {
    const int node    = 1<<0;
    const int way     = 1<<1;
    const int area    = 1<<2;
}


class SymbolList;

class Symbol { ///< Symbol
    int id;
    int textId;
    TagMap tagMap;
    Tag ndSymbolTag;
    friend class SymbolList;
public:
    Symbol() : id(invalid_sym_id), textId(invalid_sym_id) {};
    Symbol(XmlElement& symbolElement);
    int Id() const { return id; };
    int TextId() const { return textId; };
    const Tag& NdSymbolTag() const { return ndSymbolTag; };
    void print() const {
        info("id " + std::to_string(id));
        tagMap.print();
    };
};

class SymbolList ///< SymList
: public std::list<Symbol> {
public:
    void insert(Symbol& symbol);
    const Symbol& detect(const TagMap& tags);
};

class GroupList;

class Group {
    std::string name;
    TagMap keyTagsMap;
    SymbolList symbols;
    int allowedElements;
    friend class GroupList;
public:
    Group(XmlElement& groupElement);
    bool isTypeAllowed(int elemType);
};

class GroupList 
: public std::list<Group>, TrueInit {
    void insert(Group& group) { push_back(group); };
public:
    GroupList() {};
    GroupList(XmlElement& rules);
    Group * detect(const TagMap& tags, int elemType);
    const Symbol& getSymbol(const TagMap& checkedTags, int elemType);
    int getSymbolId(TagMap& checkedTags, int elemType);
};

typedef std::list<int> BackgroundList;

class Rules
: public TrueInit {
public:
    GroupList groupList;
    BackgroundList backgroundList;
    Rules() {};
    Rules(const char * rulesFileName, SymbolIdByCodeMap& symbolIds);
};

#endif // RULES_H_INCLUDED

