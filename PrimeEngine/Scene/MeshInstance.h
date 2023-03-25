#ifndef __pe_meshinstance_h__
#define __pe_meshinstance_h__

#define NOMINMAX
// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/APIAbstraction/Effect/Effect.h"

// Sibling/Children includes
#include "Mesh.h"
#include "SceneNode.h"
namespace PE {
namespace Components {

struct MeshInstance : public Component
{
	PE_DECLARE_CLASS(MeshInstance);

	// Constructor -------------------------------------------------------------
	MeshInstance(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself);

	void initFromFile(const char *assetName, const char *assetPackage,
		int &threadOwnershipMask, Matrix4x4 bbox_transformation = Matrix4x4(), bool drawbox=false);

	void initFromRegisteredAsset(const PE::Handle &h);

	virtual ~MeshInstance(){}

	virtual void addDefaultComponents();

	bool hasSkinWeights();

    bool m_culledOut;
	Handle m_hAsset;

	int m_skinDebugVertexId;
	// added
	SceneNode* scenenode_for_hw3;
	bool hw3_cull;
};

}; // namespace Components
}; // namespace PE
#endif
