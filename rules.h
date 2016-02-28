#ifndef RULES_H_INCLUDED
#define RULES_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

#include <string>
#include <map>
#include <list>
#include "xmap.h"

namespace ElemType {
    const int node    = 1<<0;
    const int way     = 1<<1;
    const int area    = 1<<2;
}

class TagMap;

class Tag {
    std::string key;
    std::string value;
    bool exist;
    friend class TagMap;
public:
    Tag() : key(""), value(""), exist(true) {};
    Tag(std::string k, std::string v, bool e=true) : key(k), value(v), exist(e) {}; 
    Tag(XmlElement& tagElement);
    const std::string& getKey() const { return key; };
    const std::string& getValue() const { return value; };
    bool empty() const { return key.empty(); };
    void print() const {
        info(key + "=" + value);
    };
};

class TagMap ///< TagList
: public std::map<std::string, Tag> {
public:
    TagMap() {};
    bool exist(const Tag& tag) const;
    bool tagsOk(const TagMap& checkedTags) const;
    void insert(Tag& tag);
    void print() const;
};

/*
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
*/
typedef std::list<int> BackgroundList;

class Rules
: public TrueInit {
public:
    //GroupList groupList;
    BackgroundList backgroundList;
    Rules() {};
    Rules(const std::string&, SymbolIdByCodeMap&);
};

#endif // RULES_H_INCLUDED

