// -*- c++ -*-
/* Generated by Together */
#ifndef TPARTITIONRELATION_H
#define TPARTITIONRELATION_H
#include "TContribution.h"
#include "bicgdefs.h"
#include <vector>
/**
 * This class describe the relationship between the partitions of a mesh
 */
class   TPartitionRelation:public TPZSaveable
{
      public:
		  virtual long    GetClassID () {
			  return TPARTITIONRELATION_ID;
		  }
	        TPartitionRelation ();
	        TPartitionRelation (int npart);
    /**
     * Returns the total number of 
     */
	static TPartitionRelation *CreateRandom (int numpart);
	int     GetNPartitions ();
	        TContribution & GetRelation (int parfrom, int parto);
	int     IncomingContribution (int partition);
    /**
     * Returns a vector containing the ID of all the partitions to each the local partitions will contribute to. 
     */
	int     OutgoingContribution (int partition);
	/**
	 * returns the processor associated with the partition
	 */
	int Processor(int partition) {
		return fProcessor[partition];
	}
	void    Print (ostream & out);
    /**
     * inform to the relation table the mesh id
     */
	void    SetMeshId (int index, OOPObjectId & meshid);
  /**
   * Packs the object in on the buffer so it can be transmitted through the network.
   * The Pack function  packs the object's class_id while function Unpack() doesn't,
   * allowing the user to identify the next object to be unpacked.
   * @param *buff A pointer to TSendStorage class to be packed.
   */
	virtual int Write(OOPStorageBuffer * buf,int classid);
  /**
   * Unpacks the object class_id
   * @param *buff A pointer to TSendStorage class to be unpacked.
   */
	virtual int Read(OOPStorageBuffer * buf,void *context);
	static TPZSaveable *Restore (OOPStorageBuffer * buf);
      private:
	int     fNumPartitions;
	/**
	 * indicates the processor where each partition resides
	 */
	vector<int> fProcessor;
	        
	vector < vector < TContribution > >fRelation;
};
#endif // TPARTITIONRELATION_H
