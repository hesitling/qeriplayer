#ifndef NERIPLAYERQT_SERVICELOCATOR_H
#define NERIPLAYERQT_SERVICELOCATOR_H

#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace NeriPlayerQt {

class ServiceLocator {
public:
    ServiceLocator() = default;
    ~ServiceLocator() = default;

    ServiceLocator(const ServiceLocator &) = delete;
    ServiceLocator &operator=(const ServiceLocator &) = delete;

    template <typename T> void registerService(std::unique_ptr<T> service)
    {
        m_services.insert_or_assign(typeid(T),
                                    ServicePtr(service.release(), [](void *ptr) { delete static_cast<T *>(ptr); }));
    }

    template <typename T> T *service() const
    {
        auto it = m_services.find(typeid(T));
        if (it == m_services.end()) {
            return nullptr;
        }

        return static_cast<T *>(it->second.get());
    }

    template <typename T> bool hasService() const
    {
        return m_services.contains(typeid(T));
    }

    void clear()
    {
        m_services.clear();
    }

private:
    using ServicePtr = std::unique_ptr<void, void (*)(void *)>;

    std::unordered_map<std::type_index, ServicePtr> m_services;
};

} // namespace NeriPlayerQt

#endif // NERIPLAYERQT_SERVICELOCATOR_H
