#include <iostream> 
#include <fstream>
#include <algorithm>
#include "yaml-cpp/null.h"
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

bool TagMap::tagsExist(const TagMap& checkedTags) const {
    auto it = checkedTags.begin();
    while (it != checkedTags.end()) {
        std::string key = it->first;
        for (auto it = checkedTags.lower_bound(key);;) {
            std::shared_ptr<Tag> p_tag = it->second;
            ++it;
            if (!(exist(*p_tag) ^ p_tag->equal)) {
                break;
            }
            else if (it == checkedTags.upper_bound(key)) {
                return false;
            }
        }
        it = checkedTags.upper_bound(key);
    }
    return true;
}

void TagMap::insert(const Tag& tag, bool as_multi) {
    iterator it = find(tag.key);
    if (it != end() && !as_multi) {
        throw Error("Tag with key '" + tag.key + "' already exist");
    }
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
        std::string s_key, s_value;
        yaml_map_it.first() >> s_key;
        if (s_key == "__dash__") {
            const YAML::Node& dash_definition = yaml_map_it.second();
            if (dash_definition.Type() == YAML::NodeType::Map) {
                for (auto it = dash_definition.begin(); it != dash_definition.end(); ++it) {
                    std::string k, v;
                    it.first() >> k;
                    it.second() >> v;
                    dashMap[id].insert(Tag(k,v));
                }
            }
            continue;
        }
        yaml_map_it.second() >> s_value;
        /// TODO parse 'not key' case
        std::stringstream ss_key(s_key);
        std::string word;
        ss_key >> word;
        bool equal = true;
        if (word == "not") {
            ss_key >> s_key;
            //equal = false;
        }
        std::stringstream ss_value(s_value);
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
            case NextWord::AS_OR:
            default:
                {
                    std::string key = s_key;
                    std::string value = word;
                    if (value == "~") { ///< is YAML null string
                        value = "";
                    }
                    idMap[key+"="+value].insert(pIdAndTagMap);
                    if (flags == NextWord::AS_NOT) {
                        //equal = false;
                    }
                    pIdAndTagMap->insert(Tag(key,value,equal),true/*as_multi*/);
                }
                break;
            }
        }
    }
    for (auto pair : *pIdAndTagMap) {
        info(pair.first+"="+pair.second->getValue());
    }
    info("\n");
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
Rules::getSymbolId(const TagMap& osm_object_tags, int elemType) {
    if (!isInited()) {
        throw Error("Rules not inited!");
    }
    //SymbolIdSet intersection;
    //bool started = false;
    for (auto it : osm_object_tags) {
        std::string k = it.second->getKey();
        std::string v = it.second->getValue();
        auto it_id_list = idMap.find(k+"="+v);
        auto _it_id_list = idMap.find(k+"=");
        if (it_id_list != idMap.end() || (it_id_list = _it_id_list) != idMap.end()) {
            const IdAndTagMapPSet& idAndTagMapPSet = it_id_list->second;
            for (auto pIdAndTagMap : idAndTagMapPSet) {
                if (osm_object_tags.tagsExist(*pIdAndTagMap)) {
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

bool
Rules::isDashPoint(const TagMap& osm_point_tags, int id) {
    if (!isInited()) {
        throw Error("Rules not inited!");
    }
    auto it = dashMap.find(id);
    if (it != dashMap.end()) {
        const TagMap& dashTags = it->second;
        return osm_point_tags.tagsExist(dashTags);
    }
    return false;
}

bool
Rules::isText(int id) {
    SymType type = RulesCpp::symbol_ids->getType(id);
    if (type == SymType::text) {
        return true;
    }
    return false;
}

