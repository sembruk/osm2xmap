#include <iostream> 
#include <fstream>
#include <algorithm>
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
        throw Error("Tag with key '" + tag.key + "' already exist");
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
    SymbolIdByCodeMap* symbol_ids;
};

/*
Symbol::Symbol(XmlElement& symbolElement)
: id(invalid_sym_id), textId(invalid_sym_id) {
    if (RulesCpp::symbolIds == nullptr) {
        throw Error("Symbols ID-code map not inited");
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
: allowedElements(ElemType::node | ElemType::way | ElemType::area) { ///< enterGroup
    name = groupElement.getAttribute<std::string>("name");
    std::string allowedElementsStr = groupElement.getAttribute<std::string>("type");
    if (!allowedElementsStr.empty()) {
        allowedElements = 0;
        if (allowedElementsStr.find('n') != std::string::npos) {
            allowedElements |= ElemType::node;
        }
        if (allowedElementsStr.find('w') != std::string::npos) {
            allowedElements |= ElemType::way;
        }
        if (allowedElementsStr.find('a') != std::string::npos) {
            allowedElements |= ElemType::area;
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
GroupList::detect(const TagMap& tags, int elemType) {
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
*/

namespace Yaml {
    const char* type(YAML::NodeType::value t) {
        switch (t) {
        case YAML::NodeType::Null:
            return "Null";
        case YAML::NodeType::Scalar:
            return "Scalar";
        case YAML::NodeType::Sequence:
            return "Sequence";
        case YAML::NodeType::Map:
            return "Map";
        }
        return "";
    }
}

void parseValueString(std::string& value) {
    /// TODO
}

void IdMap::debugPrint() {
    for (auto it : *this) {
        info("===+ "+it.first);
        for (auto id : it.second) {
            info("\t"+std::to_string(id));
        }
    }
}

enum class NextWord { AS_OR, AS_NOT };

void
Rules::parseTagMap(const YAML::Node& yaml_map, int id) {
    for (auto yaml_map_it = yaml_map.begin(); yaml_map_it != yaml_map.end(); ++yaml_map_it) {
        std::string key, value;
        yaml_map_it.first() >> key;
        yaml_map_it.second() >> value;
        /// TODO
        std::stringstream ss_value(value);
        std::string word;
        NextWord flags;
        while (ss_value >> word) {
            if (word == "or") {
                flags = NextWord::AS_OR;
                continue;
            }
            else if (word == "not") {
                flags = NextWord::AS_NOT;
                continue;
            }
            switch (flags) {
            case NextWord::AS_NOT:
                /// TODO
                break;
            case NextWord::AS_OR:
            default:
                {
                    std::string key_value(key+"="+word);
                    idMap[key_value].insert(id);
                }
                break;
            }
        }
    }
}

Rules::Rules(const std::string& rules_file_name, SymbolIdByCodeMap& symbol_ids)
: TrueInit(true) {
    RulesCpp::symbol_ids = &symbol_ids;
    std::ifstream rules_file("rules.yaml");
    YAML::Parser  parser(rules_file);
    YAML::Node    doc;
    //std::string name = rules.getAttribute<std::string>("name");
    //info("Loading rules '" + name + "'... ");

    if (parser.GetNextDocument(doc)) {
        if (doc.Type() != YAML::NodeType::Map) {
            throw Error("invalid rules file format");
        }

        if(const YAML::Node *pName = doc.FindValue("rules_name")) {
            std::string rules_name;
            *pName >> rules_name;
            if (!rules_name.empty() && rules_name != "") {
                info("Loading rules '" + rules_name + "'... ");
            }
        }
        else {
            info("Loading rules...");
        }

        const YAML::Node *pCodes = doc.FindValue("codes");
        if (pCodes->Type() != YAML::NodeType::Sequence) {
            throw Error("Codes list in the rules file not a YAML sequence");
        }
        for (YAML::Iterator it = pCodes->begin(); it != pCodes->end(); ++it) {
            YAML::Iterator code_map = it->begin();
            /// TODO check members number in list
            std::string code;
            code_map.first() >> code;
            /// TODO: parse dash symbols
            int id = RulesCpp::symbol_ids->get(code);
            const YAML::Node& symbol_definition = code_map.second();
            info(code+"\t"+std::to_string(id)+"\t"+Yaml::type(symbol_definition.Type()));
            switch (symbol_definition.Type()) {
            case YAML::NodeType::Scalar:
                {
                    std::string description;
                    symbol_definition >> description;
                    if (description == "background" || description == "bg") {
                        backgroundList.push_back(id);
                        info("background: "+code);
                    }
                }
                break;
            case YAML::NodeType::Map:
                {
                    parseTagMap(symbol_definition, id);
                }
                break;
            case YAML::NodeType::Sequence:
                {
                    for (auto it = symbol_definition.begin(); it != symbol_definition.end(); ++it) {
                        parseTagMap(*it,id);
                        /// TODO
                    }
                }
                break;
            default:
                throw Error("Illegal YAML node type");
                break;
            }
        }
    }
    idMap.debugPrint();
}

int 
Rules::getSymbolId(const TagMap& checkedTags, int elemType) {
    if (!isInited()) {
        throw Error("Rules not inited!");
    }
    SymbolIdSet intersection;
    bool started = false;
    for (auto it : checkedTags) {
        std::string kv = it.second.getKey() + "=" + it.second.getValue();
        auto it_id_list = idMap.find(kv);
        if (it_id_list != idMap.end()) {
            const SymbolIdSet& id_set = it_id_list->second;
            if (!started) {
                intersection = id_set;
                started = true;
            }
            else {
                SymbolIdSet tmp;
                std::set_intersection(id_set.begin(), id_set.end(),
                          intersection.begin(), intersection.end(),
                          std::inserter(tmp, tmp.begin()));
                intersection = tmp;
            }
            /// TODO
        }
    }
    if (!intersection.empty()) {
        auto it = intersection.begin();
        int id = *it;
        intersection.erase(it);
        if (id != 0) {
            if (!intersection.empty()) {
                warning("intersecrion size more 1");
                /*
                for (int _id : intersection) {
                    info(">>>> "+std::to_string(_id));
                }
                */
            }
            return id;
        }
    }
    return invalid_sym_id;
}

