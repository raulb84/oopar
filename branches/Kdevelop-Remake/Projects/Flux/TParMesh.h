/* Generated by Together */
#ifndef TPARMESH_H
#define TPARMESH_H

#include "fluxdefs.h"
#include "pzsave.h"
class TPZStream;
class   TParMesh:public TPZSaveable
{
public:
	TParMesh ();
	virtual ~TParMesh();
	/**
	 * Returns the class id for the current class.
	 */
	virtual int  ClassId () const {
		return TPARMESH_ID;
	}
  /**
   * Packs the object in on the buffer so it can be transmitted through the network.
   * The Pack function  packs the object's class_id while function Unpack() doesn't,
   * allowing the user to identify the next object to be unpacked.
   * @param *buff A pointer to TSendStorage class to be packed.
   */
	virtual void Write (TPZStream & buf, int withclassid=0);
  /**
   * Unpacks the object class_id
   * @param *buff A pointer to TSendStorage class to be unpacked.
   */
	virtual void Read (TPZStream & buf, void * context = 0);
	static TPZSaveable *Restore (TPZStream & buf, void * context = 0);
};
template class TPZRestoreClass<TParMesh, TPARMESH_ID>;
#endif // TPARMESH_H