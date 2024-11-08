#pragma once
#include "Script.h"

class NxContactPair;
enum eECollisionPairFlags;
class NxShape;

class GE_DLLIMPORT eCContactIterator 
{
public:
	GE_PADDING ( 0x40 );

public:
	eCContactIterator::eCContactIterator ( eCContactIterator const& );
	eCContactIterator::eCContactIterator ( NxContactPair& , bool );
	eCContactIterator::eCContactIterator ( void );
	eCContactIterator::eCContactIterator ( eEPropertySetType );
	eCContactIterator::eCContactIterator ( eEPropertySetType , bCVector const& );
	eCContactIterator::eCContactIterator ( eEPropertySetType , bCVector const& , eCCollisionShape* );
	eCContactIterator::eCContactIterator ( eEPropertySetType , bCVector const& , bCString& , GEInt );
	eCContactIterator::~eCContactIterator ( void );

	bCVector& GetAvgCollisionPosition ( void );
	bCString const& GetBone ( GEInt );
	GEInt const GetBoneIndex ( GEInt );
	eEPropertySetType GetCollisionType ( void );
	GEU32 GetExtData ( void );
	GEU32 GetNumPatches ( void );
	GEU32 GetNumPatchesRemaining ( void );
	GEU32 GetNumPoints ( void );
	GEU32 GetNumPointsRemaining ( void );
	GEU32 GetNumShapes ( void );
	eECollisionPairFlags GetPairFlags ( void );
	bCVector GetPatchNormal ( void );
	bCVector GetPoint ( void );
	NxShape* GetProprietaryShape ( GEInt );
	GEFloat GetSeparation ( void );
	eCCollisionShape* GetShape ( GEInt );
	bCVector GetShapeCenter ( GEInt );
	GEFloat GetShapeDepth ( GEInt );
	eEShapeGroup GetShapeGroup ( GEInt );
	GEFloat GetShapeHeight ( GEInt );
	bCMatrix3 GetShapeOrientation ( GEInt );
	eECollisionShapeType GetShapeType ( GEInt );
	GEFloat GetShapeWidth ( GEInt );
	bCVector const& GetSumFrictionForce ( void )const;
	bCVector const& GetSumNormalForce ( void )const;
	GEBool GoNextPatch ( void );
	GEBool GoNextPoint ( void );
	GEBool GoNextShape ( void );
	GEBool IsPointShape ( void )const;
	GEBool IsValid ( void )const;
	void ResetIterator ( void );

protected:
	void CreateIterator ( void );

};