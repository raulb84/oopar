/* Generated by Together */

#ifndef OOPDOUBLE_H
#define OOPDOUBLE_H
#include "ooppardefs.h"
#include "pzsave.h"

class OOPDouble : public TPZSaveable {
public:    
	
    double fValue;
	
    OOPDouble();
    
    void Write(TPZStream & buf, int withclassid);
	
    void Read(TPZStream & buf, void * context);
	
    virtual int ClassId() const;
	
    
};
#endif