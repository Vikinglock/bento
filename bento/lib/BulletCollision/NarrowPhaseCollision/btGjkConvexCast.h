/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  https://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_GJK_CONVEX_CAST_H
#define BT_GJK_CONVEX_CAST_H

#include "../../BulletCollision/CollisionShapes/btCollisionMargin.h"

#include "../../LinearMath/btVector3.h"
#include "btConvexCast.h"
class btConvexShape;
class btMinkowskiSumShape;
#include "btSimplexSolverInterface.h"

///GjkConvexCast performs a raycast on a convex object using support mapping.
class btGjkConvexCast : public btConvexCast
{
	btSimplexSolverInterface* m_simplexSolver;
	const btConvexShape* m_convexA;
	const btConvexShape* m_convexB;

public:
	btGjkConvexCast(const btConvexShape* convexA, const btConvexShape* convexB, btSimplexSolverInterface* simplexSolver);

	/// cast a convex against another convex object
	virtual bool calcTimeOfImpact(
		const btTransform& fromA,
		const btTransform& toA,
		const btTransform& fromB,
		const btTransform& toB,
		CastResult& result);
};

#endif  //BT_GJK_CONVEX_CAST_H
