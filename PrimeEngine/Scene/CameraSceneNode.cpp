#include "CameraSceneNode.h"
#include "../Lua/LuaEnvironment.h"
#include "PrimeEngine/Events/StandardEvents.h"
#include <cmath>
#define Z_ONLY_CAM_BIAS 0.0f
namespace PE {
namespace Components {

PE_IMPLEMENT_CLASS1(CameraSceneNode, SceneNode);

CameraSceneNode::CameraSceneNode(PE::GameContext &context, PE::MemoryArena arena, Handle hMyself) : SceneNode(context, arena, hMyself)
{
	m_near = 0.05f;
	m_far = 2000.0f;
}
void CameraSceneNode::addDefaultComponents()
{
	Component::addDefaultComponents();
	PE_REGISTER_EVENT_HANDLER(Events::Event_CALCULATE_TRANSFORMATIONS, CameraSceneNode::do_CALCULATE_TRANSFORMATIONS);
}

void CameraSceneNode::do_CALCULATE_TRANSFORMATIONS(Events::Event *pEvt)
{
	Handle hParentSN = getFirstParentByType<SceneNode>();
	if (hParentSN.isValid())
	{
		Matrix4x4 parentTransform = hParentSN.getObject<PE::Components::SceneNode>()->m_worldTransform;
		m_worldTransform = parentTransform * m_base;
	}
	
	Matrix4x4 &mref_worldTransform = m_worldTransform;

	Vector3 pos = Vector3(mref_worldTransform.m[0][3], mref_worldTransform.m[1][3], mref_worldTransform.m[2][3]);
	Vector3 n = Vector3(mref_worldTransform.m[0][2], mref_worldTransform.m[1][2], mref_worldTransform.m[2][2]);
	Vector3 target = pos + n;
	Vector3 up = Vector3(mref_worldTransform.m[0][1], mref_worldTransform.m[1][1], mref_worldTransform.m[2][1]);

	m_worldToViewTransform = CameraOps::CreateViewMatrix(pos, target, up);

	m_worldTransform2 = mref_worldTransform;

	m_worldTransform2.moveForward(Z_ONLY_CAM_BIAS);

	Vector3 pos2 = Vector3(m_worldTransform2.m[0][3], m_worldTransform2.m[1][3], m_worldTransform2.m[2][3]);
	Vector3 n2 = Vector3(m_worldTransform2.m[0][2], m_worldTransform2.m[1][2], m_worldTransform2.m[2][2]);
	Vector3 target2 = pos2 + n2;
	Vector3 up2 = Vector3(m_worldTransform2.m[0][1], m_worldTransform2.m[1][1], m_worldTransform2.m[2][1]);

	m_worldToViewTransform2 = CameraOps::CreateViewMatrix(pos2, target2, up2);
    
    PrimitiveTypes::Float32 aspect = (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getWidth()) / (PrimitiveTypes::Float32)(m_pContext->getGPUScreen()->getHeight());
    
    PrimitiveTypes::Float32 verticalFov = 0.33f * PrimitiveTypes::Constants::c_Pi_F32;
    if (aspect < 1.0f)
    {
        //ios portrait view
        static PrimitiveTypes::Float32 factor = 0.5f;
        verticalFov *= factor;
    }

	m_viewToProjectedTransform = CameraOps::CreateProjectionMatrix(verticalFov, 
		aspect,
		m_near, m_far);
	
	SceneNode::do_CALCULATE_TRANSFORMATIONS(pEvt);

	// added 
	PrimitiveTypes::Float32 hh = tan(verticalFov * 0.8 / 2.f) * m_near, hw = hh * aspect;
	Vector3 nw(-hw, hh, m_near), ne(hw, hh, m_near), se(hw, -hh, m_near), sw(-hw, -hh, m_near);
	//PEINFO("nw: %f %f %f", nw.getX(), nw.getY(), nw.getZ());
	//PEINFO("ne: %f %f %f", ne.getX(), ne.getY(), ne.getZ());
	//PEINFO("se: %f %f %f", se.getX(), se.getY(), se.getZ());
	//PEINFO("sw: %f %f %f", sw.getX(), sw.getY(), sw.getZ());
	Vector3 top = nw.crossProduct(ne),
		right = ne.crossProduct(se),
		bottom = se.crossProduct(sw),
		left = sw.crossProduct(nw),
		nearclip(0.f, 0.f, -1.f),
		farclip(0.f, 0.f, 1.f);
	/*
	PEINFO("top: %f %f %f", top.getX(), top.getY(), top.getZ());
	PEINFO("bottom: %f %f %f", bottom.getX(), bottom.getY(), bottom.getZ());
	PEINFO("left: %f %f %f", left.getX(), left.getY(), left.getZ());
	PEINFO("right: %f %f %f", right.getX(), right.getY(), right.getZ());
	PEINFO("nearclip: %f %f %f", nearclip.getX(), nearclip.getY(), nearclip.getZ());
	PEINFO("farclip: %f %f %f\n", farclip.getX(), farclip.getY(), farclip.getZ());
	*/
	// get view2world transform
	Matrix4x4 view2world = m_worldToViewTransform.inverse(),
		view2world_T = m_worldToViewTransform.transpose(); // ((M^-1)^-1)^-T = M^T
	nf_n_gbl = (view2world_T * nearclip);
	nf_n_gbl.normalize();
	nf_p_gbl = view2world * Vector3(0.f, 0.f, m_near);
	
	ff_n_gbl = view2world_T * farclip;
	ff_n_gbl.normalize();
	ff_p_gbl = view2world * Vector3(0.f, 0.f, m_far);

	const float eps = 0.000001;
	if (abs(ff_n_gbl.getX() - nf_n_gbl.getX()) < eps
		&& abs(ff_n_gbl.getY() - nf_n_gbl.getY()) < eps
		&& abs(ff_n_gbl.getZ() - nf_n_gbl.getZ()) < eps) {
		nf_n_gbl *= -1.f;
	}

	lf_n_gbl = view2world_T * left;
	lf_n_gbl.normalize();
	lf_p_gbl = view2world * Vector3();

	rf_n_gbl = view2world_T * right;
	rf_n_gbl.normalize();
	rf_p_gbl = view2world * Vector3();

	tf_n_gbl = view2world_T * top;
	tf_n_gbl.normalize();
	tf_p_gbl = view2world * Vector3();

	bf_n_gbl = view2world_T * bottom;
	bf_n_gbl.normalize();
	bf_p_gbl = view2world * Vector3();
	
	/*PEINFO("near clip normal & point: %f %f %f | %f %f %f",
		nf_n_gbl.getX(), nf_n_gbl.getY(), nf_n_gbl.getZ(), 
		nf_p_gbl.getX(), nf_p_gbl.getY(), nf_p_gbl.getZ());
	PEINFO("far clip normal & point: %f %f %f | %f %f %f",
		ff_n_gbl.getX(), ff_n_gbl.getY(), ff_n_gbl.getZ(),
		ff_p_gbl.getX(), ff_p_gbl.getY(), ff_p_gbl.getZ());
	PEINFO("left clip normal & point: %f %f %f | %f %f %f",
		lf_n_gbl.getX(), lf_n_gbl.getY(), lf_n_gbl.getZ(),
		lf_p_gbl.getX(), lf_p_gbl.getY(), lf_p_gbl.getZ());
	PEINFO("right clip normal & point: %f %f %f | %f %f %f",
		rf_n_gbl.getX(), rf_n_gbl.getY(), rf_n_gbl.getZ(),
		rf_p_gbl.getX(), rf_p_gbl.getY(), rf_p_gbl.getZ());
	PEINFO("top clip normal & point: %f %f %f | %f %f %f",
		tf_n_gbl.getX(), tf_n_gbl.getY(), tf_n_gbl.getZ(),
		tf_p_gbl.getX(), tf_p_gbl.getY(), tf_p_gbl.getZ());
	PEINFO("bottom clip normal & point: %f %f %f | %f %f %f\n",
		bf_n_gbl.getX(), bf_n_gbl.getY(), bf_n_gbl.getZ(),
		bf_p_gbl.getX(), bf_p_gbl.getY(), bf_p_gbl.getZ());*/
		
	// end added
}

}; // namespace Components
}; // namespace PE
