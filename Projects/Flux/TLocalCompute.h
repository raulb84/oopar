/* Generated by Together */

#ifndef TLOCALCOMPUTE_H
#define TLOCALCOMPUTE_H
#include "ooptask.h"
class OOPMReturnType;
class TLocalCompute : public OOPTask {
public:    

    /**
        	* Execute the task, verifying that all needed data acesses are satisfied.
        	*/
	virtual OOPMReturnType Execute ();    

    TLocalCompute(int ProcId);
};
#endif //TLOCALCOMPUTE_H