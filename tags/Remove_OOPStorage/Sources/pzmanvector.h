// Generated by Together

/**
 * @file pzmanvector.h
 * @brief Free store vector implementation.
 */
// $Id: pzmanvector.h,v 1.1 2003-10-12 15:50:12 phil Exp $

#ifndef PZMANVECTOR_H
#define PZMANVECTOR_H

#include "pzvec.h"
//#include "pzerror.h"

using namespace std;

/**
 * @ingroup util
 *
 * Implements a vector class which allows to use external storage
 * provided by the user. The external storage will be used if the
 * number of elements of the external storage is greater than or equal
 * to the number of elements the object needs. When changing the size
 * of the object, this class will only allocate new storage area if
 * the storage currently allocated is insufficient to the hold the
 * object. This makes the resize method more efficient in terms of
 * dynamic memory allocation the Shrink method will reallocate the
 * storage to fit the number of elements exactly.
 *
 * @see Shrink
 */
template < class T, int NumExtAlloc = 100 >
class TPZManVector : public TPZVec< T >
{
   public:
      /**
       * Creates a vector of a given size.
       *
       * It will call the empty constructor on all objects of type T.
       *
       * @param size Size of the new vector.
       */
      TPZManVector( const int size = 0 );

      /**
       * Creates a vector of a given size, filling it.
       *
       * It will call the empty constructor on all objects of type T
       * created. Copies the object copy to all elements.
       *
       * @param size Size of the new vector.
       * @param copy Model object to initialize the other objects.
       */
      TPZManVector( const int size, const T& copy );

      /**
       * Copy constructor.
       *
       * It will call the empty constructor on all objects of type T
       * created.
       *
       * @param copy Original vector.
       */
      TPZManVector( const TPZManVector< T, NumExtAlloc >& copy );
	 
      /**
       * Assignment operator.
       *
       * It first deletes the allocated storage before allocating
       * storage for the copy only if necessary (when there is no
       * preallocated storage or when the current storage cannot hold
       * the copied vector
       *
       * @param copy Vector which will be copied.
       * @return Reference to the current object.
       */
      TPZManVector< T, NumExtAlloc >& operator=(
	 const TPZManVector< T, NumExtAlloc >& copy);
	 
      /**
       * Destructor.
       *
       * Deletes the storage allocated.
       */
      virtual ~TPZManVector();
	 
      /** Returns number of elements allocated for this object. */
      inline int NAlloc() const { return fNAlloc; }
	
      /**
       * Expands the allocated storage to fit the newsize parameter.
       *
       * Does nothing if the externally provided storage is larger
       * than newsize. Does not change the size of the vector.
       *
       * @param newsize Storage size which is requested.
       */ 
      void Expand( const int newsize );
	
      /**
       * It reallocates storage to fit the necessary storage exactly.
       */
      void Shrink();

      /**
       * Resizes the vector object.
       *
       * It reallocates storage if necessary, and copies the existing
       * objects onto the new storage.
       *
       * @see TPZVec<T>::Resize(const int, const T&)
       *
       * @param newsize Size of the vector.
       * @param object Object used to initialize the new members.
       */
      virtual void Resize( const int newsize, const T& object );
	 
      /**
       * Resizes the vector object reallocating the storage if necessary.
       *
       * It copies the existing objects to the new storage. The new
       * members are not initialized.
       *
       * @see TPZVec<T>::Resize(const int);
       * @param newsize Size of the vector.
       */
      virtual void Resize( const int newsize );
	
   private:
      /**@shapeType DependencyLink*/
      /*#  TPZVec lnkUnnamed */

      /**
       * Returns a suggested size for expanding the storage to fit the
       * required storage.
       *
       * It suggests to expand the storage size at least 20%. This
       * method will not expand the allocated storage size.
       *
       * @param proposed Storage needed for the new vector.
       * @return Expansion size which is suggested.
       */
      int ExpandSize( const int proposed ) const;
	
      /** Number of elements allocated for this object. */
      int fNAlloc;

      /** Number of elements provided by external storage. */
      //int fNExtAlloc;

      /** Pointer to the externally allocated space. */
      T fExtAlloc[ NumExtAlloc ];
};

//--| IMPLEMENTATION |----------------------------------------------------------

template< class T, int NumExtAlloc >
TPZManVector< T, NumExtAlloc >::TPZManVector( const int size ) :
   TPZVec<T>( 0 ) // There is always some static allocation.
{
   /* If the size requested fits inside the size already provided
    * statically, provides access to that space, by setting some
    * TPZVec data.
    */
   if( size <= NumExtAlloc )
   {	
      // Needed to make TPZVec::operator[] work properly.
      fStore     = fExtAlloc;
      fNElements = size;
      // No memory was allocated by the constructor.
      fNAlloc    = 0;
   }
   else // The size requested is bigger than the size already provided.
   {
      // Executes the allocation that would be done by TPZVec<T>(size).
      fStore     = new T[ size ];
      fNElements = size;
      fNAlloc    = size;
   }
}

template< class T, int NumExtAlloc >
TPZManVector< T, NumExtAlloc >::TPZManVector( const int size, const T& copy ) :
   TPZVec<T>( 0 )
     //fNExtAlloc( NumExtAlloc ) // There is always some static allocation.
{
   /* If the size requested fits inside the size already provided
    * statically, provides access to that space, by setting some
    * TPZVec data.
    */
   if( size <= NumExtAlloc )
   {	
      // Needed to make TPZVec::operator[] work properly.
      fStore     = fExtAlloc;
      fNElements = size;
      // No memory was allocated by the constructor.
      fNAlloc    = 0;
   }
   else // The size requested is bigger than the size already provided.
   {
      // Executes the allocation that would be done by TPZVec<T>(size).
      fStore     = new T[ size ];
      fNElements = size;
      fNAlloc    = size;
   }

   for( int i = 0; i < size; i++ )
   {
      fStore[i] = copy;
   }
}

template< class T, int NumExtAlloc >
inline TPZManVector< T, NumExtAlloc >::TPZManVector(
   const TPZManVector< T, NumExtAlloc >& copy )
{
   const int size = copy.NElements();

   /* If the size requested fits inside the size already provided
    * statically, provides access to that space, by setting some
    * TPZVec data.
    */
   if( size <= NumExtAlloc )
   {	
      // Needed to make TPZVec::operator[] work properly.
      fStore     = fExtAlloc;
      fNElements = size;
      // No memory was allocated by the constructor.
      fNAlloc    = 0;
   }
   else // The size requested is bigger than the size already provided.
   {
      // Executes the allocation that would be done by TPZVec<T>(size).
      fStore     = new T[ size ];
      fNElements = size;
      fNAlloc    = size;
   }

   for( int i = 0; i < size; i++ )
   {
      fStore[i] = copy.fStore[i];
   }
}

template< class T, int NumExtAlloc >
TPZManVector< T, NumExtAlloc >& TPZManVector< T, NumExtAlloc >::operator=(
   const TPZManVector< T, NumExtAlloc >& copy )
{
   // Checking auto assignment.
   if( this == &copy )
   {
      return *this;
   }

   const int nel = copy.NElements();

   if( nel > fNAlloc && fStore && fStore != fExtAlloc )
   {
      delete [] fStore;

      fStore  = fExtAlloc;
      fNAlloc = NumExtAlloc;
   }
	
   if( fNAlloc >= nel )
   {
      fNElements = nel;
   }
   else
   {
      fStore     = new T[ nel ];
      fNAlloc    = nel;
      fNElements = nel;
   }
	
   for( int i = 0; i < nel; i++ )
   {
      fStore[i] = copy.fStore[i];
   }
	
   return *this;
}

template< class T, int NumExtAlloc >
TPZManVector< T, NumExtAlloc >::~TPZManVector()
{
   if( fStore == fExtAlloc )
   {
      fStore = 0;
   }

   //   fNExtAlloc = 0;
   fNAlloc    = 0;
}

template< class T, int NumExtAlloc >
void TPZManVector< T, NumExtAlloc >::Expand( const int newsize )
{
   // If newsize is negative then return.
   if( newsize <= fNAlloc || newsize <= NumExtAlloc )
   {
      return;
   }
   else
   {// the size is larger than the allocated memory
      T* newstore = new T[ newsize ];

      for ( int i = 0; i < fNElements; i++)
      {
	 newstore[i] = fStore[i];
      }
		
      if( fStore != fExtAlloc )
      {
	 delete [] fStore;
      }

      fStore  = newstore;
      fNAlloc = newsize;
   }
}

template< class T, int NumExtAlloc >
void TPZManVector< T, NumExtAlloc >::Shrink()
{
   // Philippe : Porque NumExtAlloc <= fNAlloc????
   //    if(fNElements <= NumExtAlloc && NumExtAlloc <= fNAlloc) {
   if (fNElements <= NumExtAlloc)
   {
      if (fStore != fExtAlloc)
      {
	 for (int i = 0; i < fNElements; i++)
	    fExtAlloc[i] = fStore[i];
				
	 if (fStore)
	    delete [] fStore;

	 fStore  = fExtAlloc;
	 fNAlloc = NumExtAlloc;
      }
   }
   else if (fNAlloc != fNElements)
   {  // then  fExtAlloc != fStore  because  NumExtAlloc != fNAlloc
      // Philippe : Memoria alocada externamente nao pode ser deletada
      //          if(fExtAlloc) delete[] fExtAlloc;
      T *newstore = 0;
		
      if (fNElements)
	 newstore = new T[fNElements];			

      for (int i = 0; i < fNElements; i++)
	 newstore[i] = fStore[i];			

      if (fStore)
	 delete[]fStore;
		
      fStore = newstore;
      fNAlloc = fNElements;

      // Philippe Isto eh um absurdo
      //          fExtAlloc = fStore;
      //          NumExtAlloc = fNAlloc;
   }
}

template< class T, int NumExtAlloc >
void TPZManVector< T, NumExtAlloc >::Resize(const int newsize, const T& object)
{
#ifndef NOTDEBUG
   if (newsize < 0)
   {
      cout << "TManVec::Resize. Bad parameter newsize." << endl;
      cout.flush ();
      return;
   }
   if (newsize == fNElements)
      return;
#endif

   if (newsize <= fNAlloc)
   {
      for (int i = fNElements; i < newsize; i++)
	 fStore[i] = object;
		
      fNElements = newsize;
   }
   else if (newsize <= NumExtAlloc)
   {  // that is, fExtAlloc != fStore. Moreover : fNElements <= fNAlloc
      // <= NumExtAlloc

      int i;

      for (i = 0; i < fNElements; i++)
      {
	 fExtAlloc[i] = fStore[i];
      }
		
      for (; i < newsize; i++)
	 fExtAlloc[i] = object;

      if(fStore != fExtAlloc) delete [] fStore;

      fStore = fExtAlloc;
      fNElements = newsize;
      fNAlloc = NumExtAlloc;
   }
   else
   {  // the size is larger than the allocated memory, then fNElements
      // is always lower than newsize because fNElemets <=fNAllocs
      int i, realsize = ExpandSize (newsize);

      T* newstore = new T[realsize];
		
      for (i = 0; i < fNElements; i++)
      {
	 newstore[i] = fStore[i];
      }
		
      for (; i < newsize; i++)
	 newstore[i] = object;
			
      if (fStore != fExtAlloc)
	 delete[]fStore;

      fStore = newstore;
      fNElements = newsize;
      fNAlloc = realsize;
   }
}

//
template< class T, int NumExtAlloc >
void TPZManVector< T, NumExtAlloc >::Resize(const int newsize)
{
#ifndef NOTDEBUG
   if (newsize < 0)
   {
      cout << "TManVec::Resize. Bad parameter newsize." << endl;
      cout.flush ();
      return;
   }
	
   if (newsize == fNElements)
      return;
#endif

   if (newsize <= fNAlloc)
   {
      fNElements = newsize;
   }
   else if (newsize <= NumExtAlloc)
   {  // that is, fExtAlloc != fStore
      for (int i = 0; i < fNElements; i++)
      {
	 fExtAlloc[i] = fStore[i];
      }

      if(fStore!=fExtAlloc)
	delete [] fStore;

      fStore     = fExtAlloc;
      fNElements = newsize;
      fNAlloc    = NumExtAlloc;
   }
   else
   {  // the size is larger than the allocated memory, then fNElements
      // is always lower than newsize because fNElemets <=fNAllocs
    
      int realsize = ExpandSize (newsize);
      T *newstore = new T[realsize];
		
      for (int i = 0; i < fNElements; i++)
      {
	 newstore[i] = fStore[i];
      }
		
      if( fStore != fExtAlloc )
	 delete [] fStore;

      fStore = newstore;
      fNElements = newsize;
      fNAlloc = realsize;
   }
}

template< class T, int NumExtAlloc >
int TPZManVector < T, NumExtAlloc >::ExpandSize (const int proposed) const
{
   return ( proposed > fNAlloc * 1.2 ? proposed : 
	    static_cast< int >( fNAlloc * 1.2 ) );
}

#endif

//--| PZ |----------------------------------------------------------------------