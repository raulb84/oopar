/* Generated by Together */

#ifndef PARCOMPRESIDUAL_H
#define PARCOMPRESIDUAL_H
#include "ooptask.h"
class OOPMReturnType;

/**
 * Implements the parallel computation of the fluxes.
 * Defines the necessary structure for the performance of flux computation in a recurrent manner.
 * This is a unique task on the parallel fluxes computation.
 * Is responsible for creating subtasks which actuate on independent partitions. 
 */
class ParCompResidual : public OOPTask {
public:	

    /**
    	* Execute the task, verifying that all needed data acesses are satisfied.
    	*/
	virtual OOPMReturnType Execute ();    


    ParCompResidual(int ProcId);    

    
private:    

    /**
     * Create tasks responsible for the computation of fluxes on independent partitions.
     * Partitios are obtained from MeTiS application on the original mesh.
     * Since the ParCompResidual is a recurrent task, the task creation will actually create the tasks only in its first run, on subsequent runs it will only actuate on the subtasks data dependence. 
     */
    void CreateFluxesTasks(int NPartitions, Solutinos * soltions, Residuals * residuals);

    /**
     * Initializes the task dependence to state variables (solutions). This is always necessary in arecurrent task. 
     */
    void InitializeSolutionVersion();

    /**
     * Holds the Ids of the subtasks created. Each subtask is responsible for computing the residual on one partition. 
     */
    vector<OOPObjectId> fSubTasksId;

    /**
     * Holds the number of partitions on the current process. 
     */
    int fNPartitions;
    vector<OOPObjectId> fRhsIds;
    vector<OOPObjectId> fSolIds;
};
#endif //PARCOMPRESIDUAL_H