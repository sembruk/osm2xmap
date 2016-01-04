#include <iostream>
#include "common.h"
#include "xml.h"

XmlElement::XmlElement(node_t * _node): node(_node) {
    name = getName();
}

bool 
XmlElement::operator==(const char* str) {
    return name == str;
}

std::string
XmlElement::getName() {
    if (node) {
        char * cName = roxml_get_name(node,NULL,0);
        name = cName;
        roxml_release(cName);
    }
    return name;
}

XmlElement 
XmlElement::getChild(std::string childName) {
    XmlElement child(roxml_get_chld(node, (char*)childName.c_str(), 0));
    return child;
}

XmlElement
XmlElement::getChild(int nb) {
    XmlElement child(roxml_get_chld(node, NULL, nb));
    return child;
}

XmlElement 
XmlElement::getChild() {
    XmlElement child(roxml_get_chld(node, NULL, 0));
    return child;
}

XmlElement&
XmlElement::operator++() {
    return (*this = XmlElement(roxml_get_next_sibling(node)));
}

class Attribute {
    char * value;
public:
    Attribute(node_t * node, std::string& attrName) {
        node_t * attr = roxml_get_attr(node,(char*)attrName.c_str(),0);
        value = roxml_get_content(attr,NULL,0,NULL);
        if (value == NULL) {
            error("no attribute named %s",attrName.c_str());
        }
    };
    ~Attribute() {
        roxml_release(value);
    }
    const char * Value() const { return value; };
};

template < >
double
XmlElement::getAttribute(std::string attrName) {
    Attribute attribute(node,attrName);
    return atof(attribute.Value());
}

template < >
int
XmlElement::getAttribute(std::string attrName) {
    Attribute attribute(node,attrName);
    return atoi(attribute.Value());
}

template < >
long
XmlElement::getAttribute(std::string attrName) {
    if (node) {
        Attribute attribute(node,attrName);
        return atol(attribute.Value());
    }
    return 0;
}

template < >
std::string
XmlElement::getAttribute(std::string attrName) {
    node_t * attr = roxml_get_attr(node,(char*)attrName.c_str(),0);
    char * value = roxml_get_content(attr,NULL,0,NULL);
    std::string ret;
    if (value != NULL) {
        ret = std::string(value);
    }
    /*
    else {
        warning("no attribute named %s",attrName.c_str());
    }
    */
    roxml_release(value);
    return ret;
}

std::string
XmlElement::getContent() {
    const int crs_desc_len = 64;
    int size;
    char str[crs_desc_len];
    roxml_get_content(node, str, crs_desc_len, &size);
    std::string content(str);
    return content;
}

int
XmlElement::getChildNumber() {
    return roxml_get_chld_nb(node);
}

node_t * 
XmlElement::addChild(const char * name) {
    return roxml_add_node(node,0,ROXML_ELM_NODE,(char*)name,NULL);
}

void
XmlElement::removeAttribute(const char * name) {
    node_t * attr = roxml_get_attr(node,(char*)name,0);
    if (attr != NULL) {
        roxml_del_node(attr);
    }
}

void
XmlElement::addAttribute(const char * name, int value) {
    removeAttribute(name);
    char valueStr[256] = {0};
    std::sprintf(valueStr,"%d",value);
    roxml_add_node(node,0,ROXML_ATTR_NODE,(char*)name,valueStr);
}

void
XmlElement::addContent(const char * text) {
    roxml_add_node(node,0,ROXML_TXT_NODE,NULL,(char*)text);
}

XmlTree::XmlTree(const char * inFilename) : XmlElement::XmlElement(roxml_load_doc((char*)inFilename)) {
    if (XmlElement::isEmpty()) {
        error("opening XML file \"%s\" failed",inFilename);
    }
}

XmlTree::~XmlTree() {
    if (!XmlElement::isEmpty()) {
        roxml_close(node);
    }
}

void 
XmlTree::saveInFile(const char * outFilename) {
    roxml_commit_changes(node,(char*)outFilename,NULL,1);
}

