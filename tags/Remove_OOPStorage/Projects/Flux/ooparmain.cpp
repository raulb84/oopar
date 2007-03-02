#include "TPartitionRelation.h"

#include "oopcommmanager.h"
#include "oopdatamanager.h"
#include "ooptaskmanager.h"
#include <iostream>
#include "TParAnalysis.h"
#include "TParCompute.h"
#include "TLocalCompute.h"
#include "TTaskComm.h"
#include "fluxdefs.h"
#include "OOPDataLogger.h"
OOPCommunicationManager *CM;
OOPDataManager *DM;
OOPTaskManager *TM;
//OOPDataLogger *LogDM;
using namespace std;
void    ParAddClass ();
int     multimain ();
int     singlemain ();
#ifdef MPI
int     mpimain (int argc, char *argv[]);
#endif

int main (int argc, char **argv)
{
/*	OOPStorageBuffer::AddClassRestore (TPARCOMPUTE_ID,
					    TParCompute::Restore);
	OOPStorageBuffer::AddClassRestore (TPARANAYSIS_ID,
					    TParAnalysis::Restore);
	OOPStorageBuffer::AddClassRestore (TLOCALCOMPUTE_ID,
					    TLocalCompute::Restore);
	OOPStorageBuffer::AddClassRestore (TTASKCOMM_ID, TTaskComm::Restore);
	*/
	// return multimain();
#ifdef MPI
	return mpimain (argc, argv);
#else
	return multimain();
#endif
}
/*int singlemain ()
{
	CM = new OOPFileComManager ("test", 1, 0);
	// CM->Initialize( argv[0], 0 );
	TM = new OOPTaskManager (CM->GetProcID ());
	DM = new OOPDataManager (CM->GetProcID ());
	TParAnalysis *paranalysis = new TParAnalysis (DM->GetProcID (), 2,1);
	paranalysis->Submit ();
	TM->Execute ();
	delete  DM;
	delete  TM;
	delete  CM;
	// cout.flush();
	return 0;
}
*/