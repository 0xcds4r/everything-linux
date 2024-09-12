#pragma once

#include <UI/Interface.h>
#include <UI/interfaces/MainSearchInterface.h>
#include <UI/interfaces/SplashInterface.h>

class UIManager
{
public:
    static void Initialise();
    static void RegisterInterfaces();
    static void Draw();

    template <typename T>
    static void RegisterInterface(const std::string& name)
    {
        static_assert(std::is_base_of<Interface, T>::value, "T must inherit from Interface");

        if (IsInterfaceRegistered(name))
        {
            std::cerr << "Interface with name \"" << name << "\" is already registered!" << std::endl;
            return;
        }

        interfaces.emplace_back(std::make_unique<T>());
        interfaceNames.emplace_back(name);

        std::cout << "Registered interface: " << name << std::endl;
    }

    template <typename T>
    static T* GetInterface(const std::string& name)
    {
        auto itName = interfaceNames.begin();
        auto itInterface = interfaces.begin();

        for (; itName != interfaceNames.end(); ++itName, ++itInterface)
        {
            if (*itName == name)
            {
                return dynamic_cast<T*>(itInterface->get());
            }
        }

        std::cerr << "Interface with name \"" << name << "\" not found!" << std::endl;
        return nullptr;
    }

    static void LoadAllInterfaces()
    {
        for (const auto& interface : interfaces)
        {
            if (interface)
            {
                interface->Display();
            }
        }
    }

    static void RenderAllInterfaces()
    {
        for (const auto& interface : interfaces)
        {
            if (interface)
            {
                interface->Render();
            }
        }
    }

    static void SetupImGuiStyle();

private:
    static bool IsInterfaceRegistered(const std::string& name)
    {
        return std::find(interfaceNames.begin(), interfaceNames.end(), name) != interfaceNames.end();
    }

    static std::list<std::unique_ptr<Interface>> interfaces;
    static std::list<std::string> interfaceNames;
};

