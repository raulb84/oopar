//
// C++ Implementation: oopaccesstag
//
// Description: 
//
//
// Author: Edimar Cesar Rylo <ecrylo@uol.com.br>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "oopaccesstag.h"

OOPAccessTag::OOPAccessTag()
{
}


OOPAccessTag::~OOPAccessTag()
{
}


bool OOPAccessTag::CanExecute()
{
  return fObjectAutoPtr;
}
TPZSaveable * OOPAccessTag::GetPointer()
{
  return fObjectAutoPtr.operator->();
}
bool OOPAccessTag::IsMyAccessTag(const OOPAccessTag & granted)
{
  if(fObjectAutoPtr) return false;
  if(!(fObjectId == granted.fObjectId)) return false; 
  if(!(fAccessMode == granted.fAccessMode)) return false; 
  if(!(fVersion== granted.fVersion)) return false; 
  if(!(fProcessor == granted.fProcessor)) return false; 
  if(!(fTaskId == granted.fTaskId)) return false;
  return true; 
}
void OOPAccessTag::Write (TPZStream  & buf)
{
  fObjectId.Write (buf);
  fTaskId.Write( buf);
  int need = fAccessMode; 
  buf.Write (&need);
  int proc = fProcessor;
  buf.Write(&proc);
  fVersion.Write(buf);
  if(fObjectAutoPtr)
  {
    fObjectAutoPtr->Write(buf, 1);
  }else
  {
    int aux = -1;
    buf.Write(&aux, 1);
  }
}
  /**
   * method to reconstruct the object
   */
void OOPAccessTag::Read (TPZStream & buf, void * context)
{
  fObjectId.Read (buf);
  fTaskId.Read(buf);
  int need = 0;
  buf.Read (&need);
  fAccessMode = (OOPMDataState) need;
  int proc = 0;
  buf.Read(&proc);
  fProcessor = proc;
  fVersion.Read(buf);
  this->fObjectAutoPtr = TPZAutoPointer<TPZSaveable>(TPZSaveable::Restore(buf, context));
}