// -*- c++ -*-
/* Generated by Together */

#ifndef TOBJECTID_H
#define TOBJECTID_H
#include <iostream>

#include "oopstorage.h"
using namespace std;
/**
 * Implements functionalities on the identification of any entity encountered
 * in the OOPar environment.
 * OOPar previous versions used simple long Identifiers.
 * Last created object on the DataManager need to be compliant to this class, in
 * the sense that fLastCreated object must keep track of an object of this type
 */
class OOPObjectId {
public:
	/**
	 * Packs itself in a buffer so it can be sent through the network.
	 * @param buf Buffer which is sent through the net.
	 */
	int Pack(OOPSendStorage *buf);
	/**
	 * Unpacks itself from a buffer so it can be received through the network.
	 * @param buf Buffer which is received through the net.
	 */
	int Unpack(OOPReceiveStorage *buf);

	/**
	 * Assign zero to ObjectId data structure
	 */
	void Zero();

	/**
	 * returns true if the id is zero
	 */
	bool IsZero() {
	  return fId == 0 && fProcId == 0;
	}
    /**
     * Simple construtor 
     */
    OOPObjectId();

    /**
     * Copy constructor 
     */
    OOPObjectId(const ::OOPObjectId & obj);    

	/**
	 * Constructor with parameters
	 * @param ProcId Initial processor Id for fProcId.
	 * @param Id Initial Id.
	 */
    OOPObjectId(long ProcId, long Id);

    /**
     * Sets object id
     * @param id: Id to be set 
     */
    void SetId(int id);

    /**
     * Sets processor id
     * @param id : id to be set 
     */
    void SetProcId(int id);

    /**
     * Returns object Id 
     */
    int GetId() const ;
	
	/**
	 * Returns Id from the object
	 */
	OOPObjectId Id();
    /**
     * Returns processor id 
     */
    int GetProcId() const;

    /**
     * Sets both object and processor id
     * @param objid: object Id to be set
     * @param procid: processor Id to be set 
     */
    void SetData(int id, int procid);

    /**
	 * Operator overloaded
	 */
	OOPObjectId & operator=(const OOPObjectId & obj);
	
	
	/**
	 * Logical operator overloaded
	 */
	bool operator == (const OOPObjectId & obj) const;
	/**
	 * Logical operator overloaded
	 */
	bool operator >= (const OOPObjectId & obj) const;
	/**
	 * Logical operator which compares and TObjectId against an int/long value.
	 * The check is made only with fId.
	 */
	 bool operator >= (int val) const;
	 bool operator < (int val) const;
    /**
     * Used for testing purposes 
     */
    static int main();

    /**
     * Prints TObjectId data 
     */
    void Print(ostream & out) const;
private:    

    /**
     * Id of object 
     */
    int fId;

    /**
     * Identifies processor Id 
     */
    int fProcId;
};
#endif //TOBJECTID_H
