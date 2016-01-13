#ifndef XML_PARSER_H_INCLUDED
#define XML_PARSER_H_INCLUDED

#ifndef __cplusplus
#error Need C++-compile to process this file
#endif

#include <string>

extern "C" {
#include "roxml.h"
}

class XmlElement {
protected:
    node_t * node;
private:
    std::string name;
    std::string getName();
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

    node_t* addChild(const char * name);
    void addAttribute(const char * name, int value);
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

