/* Generated by Together */
#include "OOPDouble.h"

OOPDouble::OOPDouble() : TPZSaveable(), fValue(0.)
{
}

int OOPDouble::ClassId() const {
    return OOPDOUBLE_ID;
}
void OOPDouble::Read(TPZStream & buf, void * context){
    TPZSaveable::Read(buf, context);
    buf.Read(&fValue);
    int clsid=0;
    buf.Read(&clsid);
    if(clsid!=ClassId()){
        cerr << "ClassId missmatch on OOPDouble::Read";
    }
}
void OOPDouble::Write(TPZStream & buf, int withclassid){
    TPZSaveable::Write(buf, withclassid);
    buf.Write(&fValue);
    int clsid = ClassId();
    buf.Write(&clsid);
}
