#ifndef RULES_H_INCLUDED
#define RULES_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

#include <string>
#include <map>
#include <list>
#include "yaml-cpp/yaml.h"
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
    //bool exist;
    friend class TagMap;
public:
    Tag() : key(""), value("")/*, exist(true)*/ {};
    Tag(std::string k, std::string v, bool e=true) : key(k), value(v)/*, exist(e)*/ {};
    //Tag(XmlElement& tagElement);
    const std::string& getKey() const { return key; };
    const std::string& getValue() const { return value; };
    bool empty() const { return key.empty(); };
    void print() const {
        info(key + "=" + value);
    };
};

typedef std::multimap<std::string, std::shared_ptr<Tag> > TagMapBase;
class TagMap ///< TagList
: public TagMapBase {
public:
    TagMap() {};
    bool exist(const Tag& tag) const;
    bool tagsExist(const TagMap& checkedTags) const;
    void insert(const Tag& tag, bool as_multi=false);
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

//typedef std::set<int> SymbolIdSet;

/*
class KvList
: public std::list<std::string> {
    int id;
public:
    KvList(int _id) : id(_id) {};
};
*/

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

class Rules
: public TrueInit {
    void parseMap(const YAML::Node& yaml_map, int id);
public:
    //GroupList groupList;
    BackgroundList backgroundList;
    IdMap idMap;
    Rules() {};
    Rules(const std::string&, SymbolIdByCodeMap&);
    int getSymbolId(const TagMap& checkedTags, int elemType);
};

#endif // RULES_H_INCLUDED

