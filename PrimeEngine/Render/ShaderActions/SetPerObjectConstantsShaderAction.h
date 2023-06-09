#ifndef __PYENGINE_2_0_SetPerObjectConstantsShaderAction_H__
#define __PYENGINE_2_0_SetPerObjectConstantsShaderAction_H__

// API Abstraction
#include "PrimeEngine/APIAbstraction/APIAbstractionDefines.h"

// Outer-Engine includes
#include <assert.h>

// Inter-Engine includes
#include "PrimeEngine/MemoryManagement/Handle.h"
#include "PrimeEngine/PrimitiveTypes/PrimitiveTypes.h"

#include "PrimeEngine/../../GlobalConfig/GlobalConfig.h"

// Sibling/Children includes
#include "ShaderAction.h"

namespace PE {

namespace Components{
struct Effect;
struct Skin;
}

struct SetPerObjectConstantsShaderAction : ShaderAction
{
	virtual ~SetPerObjectConstantsShaderAction() {}
    SetPerObjectConstantsShaderAction():m_useBones(false){}
    
	virtual void bindToPipeline(Components::Effect *pCurEffect = NULL) ;
	virtual void unbindFromPipeline(Components::Effect *pCurEffect = NULL) ;
	virtual void releaseData() ;
#if PE_PLAT_IS_PSVITA
	static void * s_pBuffer;
#	elif APIABSTRACTION_D3D11
	static ID3D11Buffer * s_pBuffer; // the buffer is first filled with data and then bound to pipeline
	// unlike DX9 where we just set registers
#	endif

    bool m_useBones;
	struct Data{
		Matrix4x4 gWVP;
		Matrix4x4 gW;
		Matrix4x4 gUnused;
		Vector4 gVertexBufferWeights;
		Matrix4x4 gWVPInverse;
		#if !APIABSTRACTION_D3D11
		Matrix4x4 gJoints[PE_MAX_BONE_COUNT_IN_DRAW_CALL];
		#endif
	} m_data;
#	if APIABSTRACTION_OGL
	// we need this struct for OGL implementation with cg
	// because we need to store a list of cgParameters in each effect (vs setting register values in DX)
	struct PerEffectBindIds
	{
#       if APIABSTRACTION_IOS
            GLuint v_gWVP, f_gWVP;
            GLuint v_gW, f_gW;
            GLuint v_gUnused, f_gUnused;
            GLuint v_gVertexBufferWeights, f_gVertexBufferWeights;
            GLuint v_gWVPInverse, f_gWVPInverse;
            GLuint v_gJoints, f_gJoints;
#       else
            CGparameter v_gWVP, f_gWVP;
            CGparameter v_gW, f_gW;
            CGparameter v_gUnused, f_gUnused;
            CGparameter v_gVertexBufferWeights, f_gVertexBufferWeights;
            CGparameter v_gWVPInverse, f_gWVPInverse;
            CGparameter v_gJoints, f_gJoints;
#       endif

		void initialize(Components::Effect *pEffect);
	};
#endif
};
}; // namespace PE
#endif
