/* Generated by Together */

#ifndef TPARTITIONRELATION_H
#define TPARTITIONRELATION_H
class TContribution;
class TPartitionRelation {
public:

    /**
     * Returns the total number of 
     */
    int GetNPartitions();

    int IncomingContribution(int partition);

    int OutgoingContribution(int partition);

private:    
    TPZMatrix<TContribution> fRelation;
};
#endif //TPARTITIONRELATION_H