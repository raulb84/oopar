/* Generated by Together */

#ifndef OOPDATALOGGER_H
#define OOPDATALOGGER_H
#include "oopsaveable.h"
#include "oopdatamanager.h"

class OOPDMOwnerTask;
class OOPDMRequestTask;
class OOPDataLogger {//: public OOPSaveable {
public:    
	void GrantAccessLog(int proc, 
						const OOPObjectId & objId,
						OOPMDataState mstate,
						const OOPDataVersion & version,
						int procorig);
	void SubmitAccessRequestLog(int proc, 
								const OOPObjectId & objId,
								OOPMDMOwnerMessageType mtype,
								OOPMDataState mstate,
								const OOPDataVersion & version,
								int procorig, const OOPObjectId & taskId);
    /**
     * Simple constructor 
     */
    OOPDataLogger();

    /**
     * Constructor with ofstream object
     * @param out output object 
     */
    OOPDataLogger(ofstream & out);

    /**
     * Constructor which takes the filename parameter
     * @param outfilename Logger will be generated on that file 
     */
    OOPDataLogger(char * logfilename);
	~OOPDataLogger();

	/**
 	 * Generates logs on the GrantAccess method of the OOPMetaData class
	 * @param town OOPDMOwnerTask object upon which the logging information will
	 * be obtained
	 * @since 16/09/2003
	 * @author longhin
 	 */
	void SendGrantAccessLog(OOPDMOwnerTask *town, int processor);

    /**
     * @param town OwnerTask to be 
     */
    void SendOwnTask(OOPDMOwnerTask *town);
    void ReceiveOwnTask(OOPDMOwnerTask *town);


	void SendReqTask(OOPDMRequestTask *req);
	void ReceiveReqTask(OOPDMRequestTask *req);

    void CancelTask(OOPMetaData &data);

private:    

    /**
     * Output stream for the logging generation
     * @since 18/09/2003 
     */
    ofstream fLogger;
};
extern OOPDataLogger * LogDM;
#endif //OOPDATALOGGER_H


