/* Generated by Together */

#ifndef TPARTITIONRELATION_H
#define TPARTITIONRELATION_H

#include "TContribution.h"
#include "oopsaveable.h"

/**
 * This class describe the relationship between the partitions of a mesh
 */
class TPartitionRelation : public OOPSaveable {
public:

	long GetClassID();
	TPartitionRelation();
  TPartitionRelation(int numpart);
    /**
     * Returns the total number of 
     */

  static TPartitionRelation *CreateRandom(int numpart);

    int GetNPartitions();

    TContribution &GetRelation(int parfrom, int parto);

    int IncomingContribution(int partition);

    int OutgoingContribution(int partition);

    void Print(ostream &out);

private:    
    int fNumPartitions;
    TContribution *fRelation;
	long fClassId;
};
#endif //TPARTITIONRELATION_H
