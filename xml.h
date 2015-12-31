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
    XmlElement(): node(NULL), name("") {};

    //XmlElement& operator=(const XmlElement& e);
    bool operator==(const char * str);

    XmlElement getChild(std::string childName);
    XmlElement getChild(int nb);
    XmlElement getChild();
    XmlElement& operator++();
    template< typename T >
    T getAttribute(std::string attrName);
    std::string getContent();
    int getChildNumber();

    bool isEmpty() { 
        if (node == NULL) 
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
    void saveInFile(const char * outFilename);
};

#endif // XML_PARSER_H_INCLUDED

