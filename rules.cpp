#include <iostream> 
#include <fstream>
#include <algorithm>
#include "rules.h"

/*
Tag::Tag(XmlElement& tagElement) {
    key   = tagElement.getAttribute<std::string>("k");
    value = tagElement.getAttribute<std::string>("v");
    exist = tagElement.getAttribute<int>("exist");
}
*/

bool TagMap::exist(const Tag& tag) const {
    const_iterator it = find(tag.key);
    if (it != end()) {
        std::shared_ptr<Tag> p_tag_in_map = it->second;
        if (tag.value == p_tag_in_map->value) {
            return true;
        }
        if (tag.value.empty() || p_tag_in_map->value.empty()) {
            return true;
        }
    }
    return false;
}

bool TagMap::tagsOk(const TagMap& checkedTags) const {
    for (const_iterator it = begin();
         it != end();
         ++it) {
        std::shared_ptr<Tag> p_tag = it->second;
        if (!checkedTags.exist(*p_tag)/* ^ tag.exist*/) {
            return false;
        }
    }
    return true;
}

void TagMap::insert(const Tag& tag, bool as_multi) {
    iterator it = find(tag.key);
    if (it != end() && !as_multi) {
        throw Error("Tag with key '" + tag.key + "' already exist");
    }
    //(*this)[tag.key] = new Tag(tag);
    std::shared_ptr<Tag> p_tag(new Tag(tag));
    TagMapBase::insert(std::make_pair(tag.key,p_tag));
}

void TagMap::print() const {
    for (const_iterator it = begin();
         it != end();
         ++it) {
        std::shared_ptr<Tag> p_tag = it->second;
        p_tag->print();
    }
}

namespace RulesCpp {
    SymbolIdByCodeMap* symbol_ids;
};

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

void IdMap::debugPrint() {
    /*
    for (auto it : *this) {
        info("===+ "+it.first);
        for (auto id : it.second) {
            info("\t"+std::to_string(id));
        }
    }
    */
}

enum class NextWord { AS_OR, AS_NOT };

void
Rules::parseMap(const YAML::Node& yaml_map, int id) {
    //KvList* pKvList = new KvList(id);
    IdAndTagMap* pIdAndTagMap = new IdAndTagMap(id);
    for (auto yaml_map_it = yaml_map.begin(); yaml_map_it != yaml_map.end(); ++yaml_map_it) {
        std::string key, value;
        yaml_map_it.first() >> key;
        yaml_map_it.second() >> value;
        /// TODO parse 'not key' case
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
                    idMap[key_value].insert(pIdAndTagMap);
                    pIdAndTagMap->insert(Tag(key,value),true/*as_multi*/);
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
                    parseMap(symbol_definition, id);
                }
                break;
            case YAML::NodeType::Sequence:
                {
                    for (auto it = symbol_definition.begin(); it != symbol_definition.end(); ++it) {
                        parseMap(*it,id);
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
    //SymbolIdSet intersection;
    //bool started = false;
    for (auto it : checkedTags) {
        std::string kv = it.second->getKey() + "=" + it.second->getValue();
        auto it_id_list = idMap.find(kv);
        if (it_id_list != idMap.end()) {
            //const KvListPSet& kvListPSet = it_id_list->second;
            const IdAndTagMapPSet& idAndTagMapPSet = it_id_list->second;
            for (auto pIdAndTagMap : idAndTagMapPSet) {
                if (pIdAndTagMap->tagsOk(checkedTags)) {
                    return pIdAndTagMap->getId();
                }
            }
            /*
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
            */
        }
    }
    /*
    if (!intersection.empty()) {
        auto it = intersection.begin();
        int id = *it;
        intersection.erase(it);
        if (id != 0) {
            if (!intersection.empty()) {
                warning("intersecrion size more 1");
                //for (int _id : intersection) {
                //    info(">>>> "+std::to_string(_id));
                //}
            }
            return id;
        }
    }
    */
    return invalid_sym_id;
}

