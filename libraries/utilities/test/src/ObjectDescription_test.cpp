
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  Embedded Machine Learning Library (EMLL)
//  File:     ObjectDescription_test.cpp (utilities)
//  Authors:  Chuck Jacobs
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ObjectDescription_test.h"

// utilities
#include "ObjectDescription.h"
#include "Serializer.h"
#include "XmlSerializer.h"

// testing
#include "testing.h"

// stl
#include <iostream>
#include <vector>
#include <sstream>

class ChildObject : public utilities::IDescribable
{
public:
    ChildObject() = default;
    ChildObject(int a, double b) : _a(a), _b(b) {}
    ChildObject(const utilities::ObjectDescription& description)
    {
        _a = description["a"].GetValue<int>();
        _b = description["b"].GetValue<double>();
    }

    static utilities::ObjectDescription GetTypeDescription()
    {
        utilities::ObjectDescription description = utilities::MakeObjectDescription<ChildObject>("Child object");
        description.AddProperty<int>("a", "Parameter a");
        description.AddProperty<double>("b", "Parameter b");
        return description;
    }

    virtual utilities::ObjectDescription GetDescription() const override
    {
        utilities::ObjectDescription description = GetTypeDescription();
        description["a"] = _a;
        description["b"] = _b;
        return description;
    }

    static std::string GetTypeName() { return "ChildObject"; }
    virtual std::string GetRuntimeTypeName() const override { return GetTypeName(); }

    int GetA() { return _a; }
    double GetB() { return _b; }

private:
    int _a;
    double _b;
};

class ParentObject : public utilities::IDescribable
{
public:
    ParentObject() = default;
    ParentObject(std::string name, int a, double b) : _name(name), _child(a, b) {}
    //ParentObject(const utilities::ObjectDescription& description)
    //{
    //}

    static utilities::ObjectDescription GetTypeDescription()
    {
        utilities::ObjectDescription description = utilities::MakeObjectDescription<ParentObject>("Parent object");
        description.AddProperty<decltype(_name)>("name", "Name");
        description.AddProperty<decltype(_child)>("child", "Child object");
        return description;
    }

    virtual utilities::ObjectDescription GetDescription() const override
    {
        utilities::ObjectDescription description = GetTypeDescription();
        description["name"] = _name;
        description["child"] = _child;
        return description;
    }

    static std::string GetTypeName() { return "ParentObject"; }
    virtual std::string GetRuntimeTypeName() const override { return GetTypeName(); }

private:
    std::string _name;
    ChildObject _child;
};

void PrintDescription(const utilities::ObjectDescription& description, size_t indentCount = 0)
{
    std::string indent(4*indentCount, ' ');
    std::cout << indent << "Type: " << description.GetObjectTypeName() << " -- " << description.GetDocumentation(); 
    if(description.HasValue())
    {
        std::cout << " = " << description.GetValueString();
    }
    std::cout << std::endl;    

    for(const auto& iter: description.Properties())
    {
        auto name = iter.first;
        auto prop = iter.second;
        PrintDescription(prop, indentCount+1);
    }
}

void TestGetTypeDescription()
{
    auto childDescription = ChildObject::GetTypeDescription();
    PrintDescription(childDescription);

    auto parentDescription = ParentObject::GetTypeDescription();
    PrintDescription(parentDescription);

    testing::ProcessTest("ObjectDescription", childDescription.HasProperty("a"));
    testing::ProcessTest("ObjectDescription", childDescription.HasProperty("b"));
    testing::ProcessTest("ObjectDescription", !childDescription.HasProperty("c"));

    testing::ProcessTest("ObjectDescription", parentDescription.HasProperty("name"));
    testing::ProcessTest("ObjectDescription", parentDescription.HasProperty("child"));
}

void TestGetObjectDescription()
{
    ChildObject childObj(3, 4.5);
    auto childDescription = childObj.GetDescription();
    PrintDescription(childDescription);

    ParentObject parentObj("Parent", 5, 6.5);
    auto parentDescription = parentObj.GetDescription();
    PrintDescription(parentDescription);

    testing::ProcessTest("ObjectDescription", childDescription.HasProperty("a"));
    testing::ProcessTest("ObjectDescription", childDescription.HasProperty("b"));
    testing::ProcessTest("ObjectDescription", !childDescription.HasProperty("c"));
    testing::ProcessTest("ObjectDescription", childDescription["a"].GetValue<int>() == 3);
    testing::ProcessTest("ObjectDescription", childDescription["b"].GetValue<double>() == 4.5);

    testing::ProcessTest("ObjectDescription", parentDescription.HasProperty("name"));
    testing::ProcessTest("ObjectDescription", parentDescription.HasProperty("child"));
    testing::ProcessTest("ObjectDescription", parentDescription.HasProperty("name"));
    testing::ProcessTest("ObjectDescription", parentDescription["name"].GetValue<std::string>() == "Parent");
    auto parentChildDescription = parentDescription["child"];
    testing::ProcessTest("ObjectDescription", parentChildDescription["a"].GetValue<int>() == 5);
    testing::ProcessTest("ObjectDescription", parentChildDescription["b"].GetValue<double>() == 6.5);
}

void TestSerializeIDescribable()
{
    utilities::SerializationContext context;
    std::stringstream strstream;
    {
        utilities::SimpleXmlSerializer serializer(strstream);
        ChildObject childObj(3, 4.5);
        serializer.Serialize("child", childObj);

        ParentObject parentObj("Parent", 5, 6.5);
        serializer.Serialize("parent", parentObj);

        // print
        std::cout << "Serialized stream:" << std::endl;
        std::cout << strstream.str() << std::endl;
    }

    utilities::SimpleXmlDeserializer deserializer(strstream);
    ChildObject deserializedChild;
    deserializer.Deserialize("child", deserializedChild, context);
    testing::ProcessTest("Deserialize IDescribable check",  deserializedChild.GetA() == 3 && deserializedChild.GetB() == 4.5f);        

    ParentObject deserializedParent;
    deserializer.Deserialize("parent", deserializedParent, context);
}