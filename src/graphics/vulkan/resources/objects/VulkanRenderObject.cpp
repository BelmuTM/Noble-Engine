#include "VulkanRenderObject.h"

Expected<void> VulkanRenderObject::create(
    const std::uint32_t    objectIndex,
    Object*                sourceObject,
    VulkanMeshManager*     meshManager,
    VulkanMaterialManager* materialManager,
    const std::uint32_t    framesInFlight
) {
    _instanceHandle = VulkanInstanceHandle{objectIndex};
    _object         = sourceObject;

    _allFramesMask = (1 << framesInFlight) - 1;
    _dirtyFrames   = _allFramesMask;

    _meshes.reserve(_object->getModel().meshes.size());

    std::size_t objectHash = 0;

    for (const Mesh& mesh : _object->getModel().meshes) {
        // Load mesh
        VulkanRenderMesh submesh{};

        submesh.mesh = meshManager->allocateMesh(mesh);

        // Load material
        VulkanMaterial* material;
        TRY_ASSIGN(material, materialManager->getOrCreateMaterial(mesh.getMaterial()));

        submesh.material = material;

        _meshes.push_back(submesh);

        // TODO: Move this somewhere else
        VulkanRenderMesh debug{};
        debug.mesh      = meshManager->allocateMesh(Mesh::makeOBBLines(mesh.getOBB()));
        debug.material  = nullptr;
        debug.drawFlags = DrawFlags::Bit::Debug;
        //_meshes.push_back(debug);

        HashUtils::combine(objectHash, MeshHash{}(mesh));

        _gpuData.debugColor = Utility::instanceColor(objectHash);
    }

    return {};
}

void VulkanRenderObject::update() {
    _gpuData.modelMatrix  = _object->getModelMatrix();
    _gpuData.normalMatrix = _object->getNormalMatrix();
}

bool VulkanRenderObject::sync() {

    if (_object->getVersion() == _syncedVersion) { return false; }
    _syncedVersion = _object->getVersion();

    update();
    markDirty();

    return true;
}
