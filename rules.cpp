#include <iostream> 
#include "rules.h"

Tag::Tag(XmlElement& tagElement) {
    key   = tagElement.getAttribute<std::string>("k");
    value = tagElement.getAttribute<std::string>("v");
    exist = tagElement.getAttribute<int>("exist");
}

bool TagMap::exist(const Tag& tag) const {
    const_iterator it = find(tag.key);
    if (it != end()) {
        Tag tagInMap = it->second;
        if (tag.value == tagInMap.value) {
            return true;
        }
        if (tag.value.empty() || tagInMap.value.empty()) {
            return true;
        }
    }
    return false;
}

bool TagMap::tagsOk(const TagMap& checkedTags) const {
    for (const_iterator it = begin();
         it != end();
         ++it) {
        Tag tag = it->second;
        if (checkedTags.exist(tag) ^ tag.exist) {
            return false;
        }
    }
    return true;
}

void TagMap::insert(Tag& tag) {
    iterator it = find(tag.key);
    if (it != end()) {
        error("Tag with key '%s' already exist",tag.key.c_str());
    }
    (*this)[tag.key] = tag;
}

void TagMap::print() const {
    for (const_iterator it = begin();
         it != end();
         ++it) {
        Tag tag = it->second;
        tag.print();
    }
}

namespace RulesCpp {
    SymbolIdByCodeMap* symbolIds;
};

Symbol::Symbol(XmlElement& symbolElement)
: id(invalid_sym_id), textId(invalid_sym_id) {
    if (RulesCpp::symbolIds == nullptr) {
        error("Symbols ID-code map not inited");
    }
    std::string code = symbolElement.getAttribute<std::string>("code");
    id = RulesCpp::symbolIds->get(code);
    std::string textCode = symbolElement.getAttribute<std::string>("textCode");
    textId = RulesCpp::symbolIds->get(textCode);
    if (id == invalid_sym_id && textId == invalid_sym_id) {
        return;
    }
    for ( XmlElement item = symbolElement.getChild();
          !item.isEmpty();
          ++item ) {
        if (item == "tag") {
            Tag tag(item);
            tagMap.insert(tag);
        }
        if (item == "ndSymbol") {
            XmlElement tag = item.getChild();
            ndSymbolTag = Tag(tag);
        }
    }
}

void
SymbolList::insert(Symbol& symbol) {
    if (symbol.id != invalid_sym_id || symbol.textId != invalid_sym_id) {
        push_back(symbol);
    }
}

const Symbol invalidSymbol;

const Symbol&
SymbolList::detect(const TagMap& tags) { ///< detectSymbol
    for (iterator it = begin();
         it != end();
         ++it) {
        Symbol& symbol = *it;
        if (symbol.tagMap.tagsOk(tags)) {
            return symbol;
        }
    }
    return invalidSymbol;
}

Group::Group(XmlElement& groupElement)
: allowedElements(int(ElemType::node) | int(ElemType::way) | int(ElemType::area)) { ///< enterGroup
    name = groupElement.getAttribute<std::string>("name");
    std::string allowedElementsStr = groupElement.getAttribute<std::string>("type");
    if (!allowedElementsStr.empty()) {
        allowedElements = 0;
        if (allowedElementsStr.find('n') != std::string::npos) {
            allowedElements |= int(ElemType::node);
        }
        if (allowedElementsStr.find('w') != std::string::npos) {
            allowedElements |= int(ElemType::way);
        }
        if (allowedElementsStr.find('a') != std::string::npos) {
            allowedElements |= int(ElemType::area);
        }
    }
    for ( XmlElement item = groupElement.getChild();
          !item.isEmpty();
          ++item ) {
        if (item == "tag") {
            Tag tag(item);
            keyTagsMap.insert(tag);
        }
        else if (item == "symbol") {
            Symbol symbol(item);
            symbols.insert(symbol);
        }
    }
}

bool Group::isTypeAllowed(int elemType) {
    if (allowedElements & elemType) {
        return true;
    }
    return false;
}

Group *
GroupList::detect(const TagMap& tags, int elemType) { ///< detectGroup
    for (iterator it = begin();
         it != end();
         ++it) {
        Group& group(*it);
        if ( group.isTypeAllowed(elemType) 
             && group.keyTagsMap.tagsOk(tags)) {
            return &(group);
        }
    }
    return nullptr;
}

const Symbol& 
GroupList::getSymbol(const TagMap& checkedTags, int elemType) {
    if (!isInited()) {
        error("Rules not inited!");
    }
    Group * group = detect(checkedTags, elemType);
    if (group != nullptr) {
        return group->symbols.detect(checkedTags);
    }
    return invalidSymbol;
}

GroupList::GroupList(XmlElement& rules)
: TrueInit(true) { /// rulesLoadGroupList()
    for ( XmlElement item = rules.getChild();
          !item.isEmpty();
          ++item ) {
        if (item == "group") {
            Group group(item);
            insert(group);
        }
    }
}

Rules::Rules(const char * rulesFileName, SymbolIdByCodeMap& symbolIds) 
: TrueInit(true) {
    RulesCpp::symbolIds = &symbolIds;
    XmlTree rulesDoc(rulesFileName);
    XmlElement rules = rulesDoc.getChild("rules");
    std::string name = rules.getAttribute<std::string>("name");
    std::cout << "Loading rules " << name << "... ";
    for ( XmlElement item = rules.getChild();
          !item.isEmpty();
          ++item ) {
        if (item == "background") {
            std::string code = item.getAttribute<std::string>("code");
            int id = RulesCpp::symbolIds->get(code);
            backgroundList.push_back(id);
        }
    }
    groupList = GroupList(rules);
    std::cout << "ok" << std::endl;
}

