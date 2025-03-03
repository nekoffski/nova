#include "MeshComposite.hh"

namespace sl {

MeshComposite::Node::Node(
  SharedPtr<Mesh> mesh, SharedPtr<Material> material, u64 depth, u64 index
) :
    mesh(mesh), material(material), name(fmt::format("Node-{}/{}", depth, index)),
    m_depth(depth), m_index(index) {
    m_instances.emplace_back();
}

std::span<Transform> MeshComposite::Node::getInstances() { return m_instances; }

Transform& MeshComposite::Node::addInstance() {
    m_instances.emplace_back();
    return m_instances.back();
}

}  // namespace sl
