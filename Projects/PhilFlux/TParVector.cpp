/* Generated by Together */
#include "TParVector.h"
#include "fluxdefs.h"
TParVector::TParVector ()
{
}
long TParVector::GetClassID ()
{
	return TPARVECTOR_ID;
}
int TParVector::Pack (OOPSendStorage * buf)
{
	OOPSaveable::Pack(buf);
//	buf->PkLong (TPARVECTOR_ID);
	vector < double >::iterator i = fData.begin ();
	int aux = fData.size ();
	buf->PkInt (&aux);
	while (i != fData.end ()) {
		buf->PkDouble (&(*i), 1);
	}
//	OOPSaveable::Pack (buf);
	return 1;
}
int TParVector::Unpack (OOPReceiveStorage * buf)
{
	OOPSaveable::Unpack(buf);
	int aux = 0;
	int i = 0;
	buf->UpkInt (&aux);
	double val = 0.;
	for (i = 0; i < aux; i++) {
		buf->UpkDouble (&val, 1);
		fData.push_back (val);
	}
//	OOPSaveable::Unpack (buf);
	return 1;
}
void TParVector::SetVector (vector < double >data)
{
#ifndef WIN32
#warning "Data should be changed to something else"
#endif
	fData = data;
}
void TParVector::Resize (int size)
{
	fData.resize (size);
}
OOPSaveable *TParVector::Restore (OOPReceiveStorage * buf) {
	TParVector *loc = new TParVector();
	loc->Unpack(buf);
	return loc;
}
