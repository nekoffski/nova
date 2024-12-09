#pragma once

#include <vector>
#include <span>

#include "starlight/core/math/Transform.hh"
#include "starlight/core/utils/Resource.hh"

#include "gpu/Mesh.hh"
#include "Material.hh"

namespace sl {

class MeshComposite {
public:
    class Node {
        friend class MeshComposite;

    public:
        explicit Node(
          ResourceRef<Mesh> mesh, ResourceRef<Material> material, u64 depth,
          u64 index
        );

        ResourceRef<Mesh> getMesh();
        ResourceRef<Material> getMaterial();
        const std::string& getName() const;
        std::span<Transform> getInstances();

        Transform& addInstance();

    private:
        template <typename C>
        requires Callable<C, void, Node&>
        void traverse(C& callback) {
            callback(*this);
            for (auto& node : m_children) node.traverse(callback);
        }

        ResourceRef<Mesh> m_mesh;
        ResourceRef<Material> m_material;

        u64 m_depth;
        u64 m_index;

        std::string m_name;

        std::vector<Transform> m_instances;
        std::vector<Node> m_children;
    };

    explicit MeshComposite(ResourceRef<Mesh> mesh, ResourceRef<Material> material) :
        m_root(mesh, material, 0u, 0u) {}

    Node& getRoot() { return m_root; }

    template <typename C>
    requires Callable<C, void, Node&>
    void traverse(C&& callback) {
        m_root.traverse(callback);
    }

private:
    Node m_root;
};

}  // namespace sl
