/* Generated by Together */

#include "OOPDataLogger.h"

using namespace std;
class OOPDMOwnerTask;
OOPDataLogger::OOPDataLogger(){
	//Initialize ostream pointer
}
OOPDataLogger::OOPDataLogger(ofstream & out){
    //fLogger=out;
}
OOPDataLogger::OOPDataLogger(char * logfilename){
	fLogger.open(logfilename);
	
} 
OOPDataLogger::~OOPDataLogger(){
	fLogger.flush();
	fLogger.close();
}

void OOPDataLogger::LogReleaseAccess(int proc, OOPObjectId & Id, OOPMDataState state, int targetproc, OOPObjectId & taskId){
	fLogger << proc << "\tId " << Id << "\tReleasing access ";
	switch (state )
	{
		case  ENoAccess:
			fLogger << "ENoAccess\t";
			break;
		case  EReadAccess:
			fLogger << "EReadAccess\t";
			fLogger << "From processor " << targetproc << endl;
			break;
		case  EWriteAccess:
			fLogger << "EWriteAccess\t";
			fLogger << "From task " << taskId << endl;
			break;
		case  EVersionAccess:
			fLogger << "EVersionAccess\t";
			fLogger << "From task " << taskId << endl;
			break;
	}
	
}
void OOPDataLogger::LogSetVersion(int proc, OOPObjectId & Id, OOPDataVersion & oldver,
					const OOPDataVersion & newver){
	fLogger << proc << "\tId " << Id << "\tSetting old version "
						<< oldver << "\tTo new version " << newver << endl;
}
void OOPDataLogger::LogGeneric(int proc, OOPObjectId & Id, char * msg){
	fLogger << proc;
	fLogger << "\tId " << Id << "\t";
	fLogger << msg << endl;
}
void OOPDataLogger::SendGrantAccessLog(OOPDMOwnerTask *town, int processor){
	town->LogMe(fLogger);
	fLogger << endl;
	fLogger.flush();
}
void OOPDataLogger::GrantAccessLog(int proc, 
									const OOPObjectId & objId,
									OOPMDataState mstate,
									const OOPDataVersion & version,
									int procorig){
	//town->LogMe(fLogger);
	fLogger << proc << "\t";
	fLogger << "Id " << objId << "\t";
	fLogger << "Granting ";
	//fLogger << "type ";
	/*
	ENoMessage,
	ECancelReadAccess,
	ECancelReadAccessConfirmation,
	ESuspendAccess,
	ESuspendAccessConfirmation,
	ESuspendSuspendAccess,
	ETransferOwnership,
	EGrantReadAccess,
	EGrantVersionAccess,
	ENotifyDeleteObject,
	
	
	
	switch (mtype)
	{
		case  ENoMessage:
			fLogger << "ENoMessage\t";
			break;
		case  ECancelReadAccess:
			fLogger << "ECancelReadAccess\t";
			break;
		case  ECancelReadAccessConfirmation:
			fLogger << "ECancelReadAccessConfirmation\t";
			break;
		case  ESuspendAccess:
			fLogger << "ESuspendAccess\t";
			break;
		case  ESuspendAccessConfirmation:
			fLogger << "ESuspendAccessConfirmation\t";
			break;
		case  ESuspendSuspendAccess:
			fLogger << "ESuspendSuspendAccess\t";
			break;
		case  ETransferOwnership:
			fLogger << "ETransferOwnership\t";
			break;
		case  EGrantReadAccess:
			fLogger << "EGrantReadAccess\t";
			break;
		case  EGrantVersionAccess:
			fLogger << "EGrantVersionAccess\t";
			break;
		case  ENotifyDeleteObject:
			fLogger << "ENotifyDeleteObject\t";
			break;
		defaults:
			fLogger << "Uninitialized fType property\t";
			break;
	}
	fLogger << "State ";
	
	ENoAccess,
	EReadAccess,
	EWriteAccess,
	EVersionAccess

	*/
	switch (mstate )
	{
		case  ENoAccess:
			fLogger << "ENoAccess\t";
			break;
		case  EReadAccess:
			fLogger << "EReadAccess\t";
			break;
		case  EWriteAccess:
			fLogger << "EWriteAccess\t";
			break;
		case  EVersionAccess:
			fLogger << "EVersionAccess\t";
			break;
	}
	
	fLogger << "Version " << version << "\t";
	fLogger << "ProcOrigin " << procorig;
	fLogger << "\n";
	fLogger.flush();
}
void OOPDataLogger::GrantAccessLog(int proc, 
									const OOPObjectId & objId,
									OOPMDataState mstate,
									const OOPDataVersion & version,
									int procorig,
									OOPObjectId & taskId){
	//town->LogMe(fLogger);
	fLogger << proc << "\t";
	fLogger << "Id " << objId << "\t";
	fLogger << "Granting ";
	//fLogger << "type ";
	/*
	ENoMessage,
	ECancelReadAccess,
	ECancelReadAccessConfirmation,
	ESuspendAccess,
	ESuspendAccessConfirmation,
	ESuspendSuspendAccess,
	ETransferOwnership,
	EGrantReadAccess,
	EGrantVersionAccess,
	ENotifyDeleteObject,
	
	
	
	switch (mtype)
	{
		case  ENoMessage:
			fLogger << "ENoMessage\t";
			break;
		case  ECancelReadAccess:
			fLogger << "ECancelReadAccess\t";
			break;
		case  ECancelReadAccessConfirmation:
			fLogger << "ECancelReadAccessConfirmation\t";
			break;
		case  ESuspendAccess:
			fLogger << "ESuspendAccess\t";
			break;
		case  ESuspendAccessConfirmation:
			fLogger << "ESuspendAccessConfirmation\t";
			break;
		case  ESuspendSuspendAccess:
			fLogger << "ESuspendSuspendAccess\t";
			break;
		case  ETransferOwnership:
			fLogger << "ETransferOwnership\t";
			break;
		case  EGrantReadAccess:
			fLogger << "EGrantReadAccess\t";
			break;
		case  EGrantVersionAccess:
			fLogger << "EGrantVersionAccess\t";
			break;
		case  ENotifyDeleteObject:
			fLogger << "ENotifyDeleteObject\t";
			break;
		defaults:
			fLogger << "Uninitialized fType property\t";
			break;
	}
	fLogger << "State ";
	
	ENoAccess,
	EReadAccess,
	EWriteAccess,
	EVersionAccess

	*/
	switch (mstate )
	{
		case  ENoAccess:
			fLogger << "ENoAccess\t";
			break;
		case  EReadAccess:
			fLogger << "EReadAccess\t";
			break;
		case  EWriteAccess:
			fLogger << "EWriteAccess\t";
			break;
		case  EVersionAccess:
			fLogger << "EVersionAccess\t";
			break;
	}
	
	fLogger << "Version " << version << "\t";
	fLogger << "To Task " << taskId << "\t";
	fLogger << "ProcOrigin " << procorig;
	fLogger << "\n";
	fLogger.flush();
}

void OOPDataLogger::SendOwnTask(OOPDMOwnerTask *town){
	town->LogMe(fLogger);
	fLogger << endl;
	fLogger.flush();
}
void OOPDataLogger::ReceiveOwnTask(OOPDMOwnerTask *town){
	town->LogMeReceived(fLogger);
	fLogger << endl;
	fLogger.flush();
}
void OOPDataLogger::SendReqTask(OOPDMRequestTask *req){
	req->LogMe(fLogger);
	fLogger.flush();
}
void OOPDataLogger::SubmitAccessRequestLog(int proc, 
									const OOPObjectId & objId,
									OOPMDMOwnerMessageType mtype,
									OOPMDataState mstate,
									const OOPDataVersion & version,
									int procorig, const OOPObjectId & taskId){
	fLogger << proc << "\t";
	fLogger << "Id " << objId;
	fLogger << "\tSubmitting ";
	/*
	ENoMessage,
	ECancelReadAccess,
	ECancelReadAccessConfirmation,
	ESuspendAccess,
	ESuspendAccessConfirmation,
	ESuspendSuspendAccess,
	ETransferOwnership,
	EGrantReadAccess,
	EGrantVersionAccess,
	ENotifyDeleteObject,
	
	
	
	switch (mtype)
	{
		case  ENoMessage:
			fLogger << "ENoMessage\t";
			break;
		case  ECancelReadAccess:
			fLogger << "ECancelReadAccess\t";
			break;
		case  ECancelReadAccessConfirmation:
			fLogger << "ECancelReadAccessConfirmation\t";
			break;
		case  ESuspendAccess:
			fLogger << "ESuspendAccess\t";
			break;
		case  ESuspendAccessConfirmation:
			fLogger << "ESuspendAccessConfirmation\t";
			break;
		case  ESuspendSuspendAccess:
			fLogger << "ESuspendSuspendAccess\t";
			break;
		case  ETransferOwnership:
			fLogger << "ETransferOwnership\t";
			break;
		case  EGrantReadAccess:
			fLogger << "EGrantReadAccess\t";
			break;
		case  EGrantVersionAccess:
			fLogger << "EGrantVersionAccess\t";
			break;
		case  ENotifyDeleteObject:
			fLogger << "ENotifyDeleteObject\t";
			break;
		defaults:
			fLogger << "Uninitialized fType property\t";
			break;
	}
	fLogger << "State ";
	
	ENoAccess,
	EReadAccess,
	EWriteAccess,
	EVersionAccess

	*/
	switch (mstate )
	{
		case  ENoAccess:
			fLogger << "ENoAccess\t";
			break;
		case  EReadAccess:
			fLogger << "EReadAccess\t";
			break;
		case  EWriteAccess:
			fLogger << "EWriteAccess\t";
			break;
		case  EVersionAccess:
			fLogger << "EVersionAccess\t";
			break;
	}
	
	fLogger << "Version " << version << "\t";
	fLogger << "To task " << taskId;
	fLogger << "\n";
	fLogger.flush();
}

void OOPDataLogger::ReceiveReqTask(OOPDMRequestTask *req){
	req->LogMe(fLogger);
	fLogger << "\t\treceived\n";
	fLogger.flush();
}

