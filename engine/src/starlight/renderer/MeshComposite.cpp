#include "MeshComposite.hh"

namespace sl {

MeshComposite::Node::Node(
  ResourceRef<Mesh> mesh, ResourceRef<Material> material, u64 depth, u64 index
) :
    m_mesh(mesh), m_material(material), m_depth(depth), m_index(index),
    m_name(fmt::format("Node-{}/{}", depth, index)) {
    m_instances.emplace_back();
}

ResourceRef<Mesh> MeshComposite::Node::getMesh() { return m_mesh; }

ResourceRef<Material> MeshComposite::Node::getMaterial() { return m_material; }

const std::string& MeshComposite::Node::getName() const { return m_name; }

std::span<Transform> MeshComposite::Node::getInstances() { return m_instances; }

Transform& MeshComposite::Node::addInstance() {
    m_instances.emplace_back();
    return m_instances.back();
}

}  // namespace sl
