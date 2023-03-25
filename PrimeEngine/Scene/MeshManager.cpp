// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

#include "MeshManager.h"
// Outer-Engine includes

// Inter-Engine includes
#include "PrimeEngine/FileSystem/FileReader.h"
#include "PrimeEngine/APIAbstraction/GPUMaterial/GPUMaterialSet.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"
#include "PrimeEngine/APIAbstraction/Texture/Texture.h"
#include "PrimeEngine/APIAbstraction/Effect/EffectManager.h"
#include "PrimeEngine/APIAbstraction/GPUBuffers/VertexBufferGPUManager.h"
#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"

#include "PrimeEngine/Geometry/SkeletonCPU/SkeletonCPU.h"

#include "PrimeEngine/Scene/RootSceneNode.h"

#include "Light.h"

// Sibling/Children includes

#include "MeshInstance.h"
#include "Skeleton.h"
#include "SceneNode.h"
#include "DrawList.h"
#include "SH_DRAW.h"
#include "PrimeEngine/Lua/LuaEnvironment.h"
#include "DebugRenderer.h"

namespace PE {
namespace Components{

PE_IMPLEMENT_CLASS1(MeshManager, Component);
MeshManager::MeshManager(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself)
	: Component(context, arena, hMyself)
	, m_assets(context, arena, 256)
{
}

PE::Handle MeshManager::getAsset(const char *asset, const char *package, int &threadOwnershipMask, Matrix4x4 bbox_transformation /*pMainSN->m_base*/,
	bool drawbox/*=false*/)
{
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "%s/%s", package, asset);
	
	int index = m_assets.findIndex(key);
	if (index != -1) // if asset is already loaded (key is in m_asset), than return the stored handle inside m_asset
	{
		// added 
		
		if (drawbox) {
			Mesh* msh = (Mesh*)m_assets.m_pairs[index].m_value.getObject();
			PositionBufferCPU* pbcu0 = (PositionBufferCPU*)msh->m_hPositionBufferCPU.getObject();
			DebugRenderer::Instance()->createBoundingBoxMesh(pbcu0->dim1_min, pbcu0->dim1_max,
				pbcu0->dim2_min, pbcu0->dim2_max,
				pbcu0->dim3_min, pbcu0->dim3_max, bbox_transformation);
		}
		// end added
		return m_assets.m_pairs[index].m_value;
	}
	Handle h;

	if (StringOps::endswith(asset, "skela"))
	{
		PE::Handle hSkeleton("Skeleton", sizeof(Skeleton));
		Skeleton *pSkeleton = new(hSkeleton) Skeleton(*m_pContext, m_arena, hSkeleton);
		pSkeleton->addDefaultComponents();

		pSkeleton->initFromFiles(asset, package, threadOwnershipMask);
		h = hSkeleton;
	}
	else if (StringOps::endswith(asset, "mesha"))
	{
		MeshCPU mcpu(*m_pContext, m_arena);
		mcpu.ReadMesh(asset, package, "");
		
		PE::Handle hMesh("Mesh", sizeof(Mesh));
		Mesh *pMesh = new(hMesh) Mesh(*m_pContext, m_arena, hMesh);
		pMesh->addDefaultComponents();

		pMesh->loadFromMeshCPU_needsRC(mcpu, threadOwnershipMask);
		// added
		if (drawbox) {
			PositionBufferCPU* pbcu_ptr = (PositionBufferCPU*)mcpu.m_hPositionBufferCPU.getObject();
			
			DebugRenderer::Instance()->createBoundingBoxMesh(pbcu_ptr->dim1_min, pbcu_ptr->dim1_max,
				pbcu_ptr->dim2_min, pbcu_ptr->dim2_max,
				pbcu_ptr->dim3_min, pbcu_ptr->dim3_max, bbox_transformation);
		}
		/*
		float databuf[24];
		// draw a line
		// pt1
		databuf[0] = 0.f;
		databuf[1] = 0.f;
		databuf[2] = 0.f;
		databuf[3] = 1.f;
		databuf[4] = 1.f;
		databuf[5] = 1.f;
		// pt2
		databuf[6] = 1.f;
		databuf[7] = 1.f;
		databuf[8] = 1.f;
		databuf[9] = 1.f;
		databuf[10] = 1.f;
		databuf[11] = 1.f;
		// pt3
		databuf[12] = 1.f;
		databuf[13] = 0.f;
		databuf[14] = 1.f;
		databuf[15] = 1.f;
		databuf[16] = 1.f;
		databuf[17] = 1.f;
		// pt4
		databuf[18] = 1.f;
		databuf[19] = 1.f;
		databuf[20] = 1.f;
		databuf[21] = 1.f;
		databuf[22] = 0.f;
		databuf[23] = 1.f;*/
		//DebugRenderer::Instance()->createLineMesh(false, Matrix4x4(), databuf, 4, 100000.0f, 1.0f);

		// end added

#if PE_API_IS_D3D11
		// todo: work out how lods will work
		//scpu.buildLod();
#endif
        // generate collision volume here. or you could generate it in MeshCPU::ReadMesh()
        pMesh->m_performBoundingVolumeCulling = true; // will now perform tests for this mesh

		h = hMesh;
	}


	PEASSERT(h.isValid(), "Something must need to be loaded here");

	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
	return h;
}

void MeshManager::registerAsset(const PE::Handle &h)
{
	static int uniqueId = 0;
	++uniqueId;
	char key[StrTPair<Handle>::StrSize];
	sprintf(key, "__generated_%d", uniqueId);
	
	int index = m_assets.findIndex(key);
	PEASSERT(index == -1, "Generated meshes have to be unique");
	
	RootSceneNode::Instance()->addComponent(h);
	m_assets.add(key, h);
}

}; // namespace Components
}; // namespace PE
