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

#ifndef XML_PARSER_H_INCLUDED
#define XML_PARSER_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

#include <string>

extern "C" {
#include <roxml.h>
}

class XmlElement {
protected:
    node_t * node;
private:
    std::string name;
    std::string getName();

    template < typename T >
    void _addAttribute(const char * name, T value, std::true_type) {
        roxml_add_node(node,0,ROXML_ATTR_NODE,(char*)name,(char*)std::to_string(value).c_str());
    }
    template < typename T >
    void _addAttribute(const char * name, T value, std::false_type) {
        roxml_add_node(node,0,ROXML_ATTR_NODE,(char*)name,(char*)std::string(value).c_str());
    }
public:
    XmlElement(node_t * _node);
    XmlElement(const XmlElement& e): node(e.node), name(e.name) {};
    XmlElement(): node(nullptr), name("") {};

    bool operator==(const char * str);

    XmlElement getChild(const std::string childName) const;
    XmlElement getChild(int nb) const;
    XmlElement getChild() const;
    XmlElement& operator++();
    template< typename T >
    T getAttribute(const std::string attrName) const;
    const std::string getContent() const;
    int getChildNumber() const;

    bool isEmpty() const { 
        if (node == nullptr) 
            return true; 
        else 
            return false;
    };

    node_t* addChildToBegin(const char * name);
    node_t* addChildToEnd(const char * name);
    node_t* addChild(const char * name);
    void removeChild(const char * name);
    template< typename T >
    void addAttribute(const char * name, T value) {
        removeAttribute(name);
        _addAttribute(name, value, std::is_arithmetic<T>());
    }
    void removeAttribute(const char * name);
    void addContent(const char * text);
};

class XmlTree
: public XmlElement {
public:
    XmlTree(const char * inFilename);
    ~XmlTree();
    void saveInFile(const char * outFilename, bool humanReadable = true);
};

#endif // XML_PARSER_H_INCLUDED

