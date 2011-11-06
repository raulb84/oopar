/***************************************************************************
 *   Copyright (C) 2006 by Philippe Devloo   *
 *   phil@fec.unicamp.br   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "tpzgensubstruct.h"
#include "pzgengrid.h"
#include "pzgmesh.h"
#include "pzcmesh.h"
#include "pzpoisson3d.h"
#include "pzbndcond.h"
#include "TPZGeoElement.h"
#include "pzgeopoint.h"
#include "pzrefpoint.h"
#include "pzsubcmesh.h"

#include "tpznodesetcompute.h"
#include "pzmetis.h"

#include "tpzdohrmatrix.h"
#include "tpzdohrsubstruct.h"

#include "pzanalysis.h"

#include "pzskylstrmatrix.h"

#include "pzsubcmesh.h"

#include <sstream>
#include "pzlog.h"

#ifdef LOG4CXX
static LoggerPtr logger(Logger::getLogger("substruct.gensubstruct"));
#endif


TPZGenSubStruct::TPZGenSubStruct(int dimension, int numlevels, int substructlevel) : fMatDist(DistMaterial),fK(8,1.),
                                 fDimension(dimension),
  fNumLevels(numlevels),fSubstructLevel(substructlevel)
{
}


TPZGenSubStruct::~TPZGenSubStruct()
{
}

REAL co[8][3] = {
  {0.,0.,0.},
  {1.,0.,0.},
  {1.,1.,0.},
  {0.,1.,0.},
  {0.,0.,1.},
  {1.,0.,1.},
  {1.,1.,1.},
  {0.,1.,1.}
};
    /// method which will generate the computational mesh
void TPZGenSubStruct::GenerateMesh()
{
  std::cout << "Generating mesh\n";
  TPZGeoMesh *gmesh = new TPZGeoMesh;
  if(fDimension == 2)
  {
    TPZVec<int> nx(2,1);
    TPZVec<REAL> x0(2,0.),x1(2,1.);
    TPZGenGrid gen(nx,x0,x1,1,0.);
    gen.Read(*gmesh);
  } else if(fDimension == 3)
  {
    int ic;
    gmesh->NodeVec().Resize(8);
    TPZVec<REAL> noco(3);
    TPZVec<int> nodeindices(8);
    for(ic=0; ic<8; ic++)
    {
      nodeindices[ic] = ic;
      int i;
      for(i=0; i<3; i++)
      {
        noco[i] = co[ic][i];
      }
      gmesh->NodeVec()[ic].Initialize(noco,*gmesh);
    }
    int matid = 1;
    int index;
    gmesh->CreateGeoElement(ECube,nodeindices,matid,index);
  }
  this->fCMesh = new TPZCompMesh(gmesh);
  TPZVec<int> nodeindices(1,0);
  new TPZGeoElement<pzgeom::TPZGeoPoint,pzrefine::TPZRefPoint> (nodeindices,-1,*gmesh);
  TPZVec<REAL> convdir(fDimension,1.);
  TPZMatPoisson3d *matp;
  int imat;
  for(imat=0; imat<fK.NElements(); imat++)
  {
    matp = new TPZMatPoisson3d(imat+1,fDimension);
    matp->SetInternalFlux(1.);
    matp->SetParameters(fK[imat],0.,convdir);
    {
      TPZAutoPointer<TPZMaterial> mat (matp);
      fCMesh->InsertMaterialObject(mat);
    }
  }
  
  TPZAutoPointer<TPZMaterial> mat (fCMesh->FindMaterial(1));
  TPZFMatrix val1(1,1,0.),val2(1,1,0.);
  TPZBndCond *bc = new TPZBndCond(mat,-1,0,val1,val2);
  TPZAutoPointer<TPZMaterial> matbc(bc);
  fCMesh->InsertMaterialObject(matbc);
  gmesh->BuildConnectivity();
  UniformRefine();
  fCMesh->AutoBuild();
#ifdef LOG4CXX
  {
    std::stringstream str;
    fCMesh->Print(str);
    LOGPZ_DEBUG(logger,str.str());
  }
#endif
  std::cout << "Substructuring the mesh\n";
  SubStructure();
#ifdef LOG4CXX
  {
    std::stringstream str;
    fCMesh->Print(str);
    LOGPZ_DEBUG(logger,str.str());
  }
#endif
  std::cout << "Identifying corner nodes\n";
  IdentifyCornerNodes();
}

    /// divide the geometric elements till num levels is achieved
void TPZGenSubStruct::UniformRefine()
{
  TPZGeoMesh *gmesh = fCMesh->Reference();
  int il;
  for(il=0; il<fNumLevels; il++)
  {
    int nel = gmesh->NElements();
    int iel;
    int nk = fK.NElements();
    for(iel=0; iel<nel; iel++)
    {
      TPZGeoEl *gel = gmesh->ElementVec()[iel];
      if(gel->Level() < fNumLevels && !gel->HasSubElement() && gel->Dimension() > 0)
      {
        TPZVec<TPZGeoEl *> subel(4);
        gel->Divide(subel);
        if(fMatDist == DistMaterial)
        {
          if(il == 0 && gel->MaterialId() == 1)
          {
            int nsub = subel.NElements();
            int is;
            for(is=0; is<nsub; is++) 
            {
              subel[is]->SetMaterialId(1+is%nk);
            }
          }
        } else
        {
          if(gel->MaterialId() > 0)
          {
            int nsub = subel.NElements();
            int is;
            for(is=0; is<nsub; is++) 
            {
              subel[is]->SetMaterialId(1+(rand()%nk));
            }
          }
        }
      }
    }
  }
}

// divide the elements in substructures
void TPZGenSubStruct::SubStructure()
{
  TPZGeoMesh *gmesh = fCMesh->Reference();
  int nel = gmesh->NElements();
  int iel;
  for(iel=0; iel<nel; iel++)
  {
    TPZGeoEl *gel = gmesh->ElementVec()[iel];
    if(gel->Level() == this->fSubstructLevel)
    {
      TPZStack<TPZCompElSide> subels;
      TPZGeoElSide gelside(gel,gel->NSides()-1);
      gmesh->ResetReference();
      fCMesh->LoadReferences();
      gelside.HigherLevelCompElementList2(subels,0,0);
      int nelstack = subels.NElements();
		if(!nelstack)
		{
			TPZCompElSide celside = gelside.Reference();
			if(celside.Element()) 
			{
				subels.Push(celside);
				nelstack = 1;
			}
		}
      int index;
      TPZCompMesh *cmesh = fCMesh.operator->();
      TPZSubCompMesh *submesh = new TPZSubCompMesh(*cmesh,index);
      int sub;
      for(sub=0; sub<nelstack; sub++)
      {
        if(subels[sub].Reference().Dimension() == fDimension)
        {
          submesh->TransferElement(cmesh,subels[sub].Element()->Index());
        }
      }      
    }
  }
  // transfer the point load
  // find the point elements
  nel = fCMesh->NElements();
  TPZCompEl *celpoint = 0;
  for(iel=0; iel<nel; iel++)
  {
    TPZCompEl *cel = fCMesh->ElementVec()[iel];
    if(!cel) continue;
    if(cel->Dimension() == 0) 
    {
      celpoint = cel;
      break;
    }
  }
  if(celpoint)
  {
    int conindex = celpoint->ConnectIndex(0);
    for(iel=0; iel<nel; iel++)
    {
      TPZCompEl *cel = fCMesh->ElementVec()[iel];
      if(!cel) continue;
      TPZSubCompMesh *submesh = dynamic_cast<TPZSubCompMesh *>(cel);
      if(!submesh) continue;
      int nc = submesh->NConnects();
      int ic;
      for(ic=0; ic<nc; ic++)
      {
        if(submesh->ConnectIndex(ic) == conindex)
        {
          submesh->TransferElement(fCMesh.operator->(),celpoint->Index());
          celpoint = 0;
          conindex = -1;
          break;
        }
      }
      if(!celpoint) break;
    }
  }
//#define MAKEINTERNAL
#ifdef MAKEINTERNAL
	// make all nodes internal
	nel = fCMesh->NElements();
	for(iel=0; iel<nel; iel++)
	{
		TPZCompEl *cel = fCMesh->ElementVec()[iel];
		if(!cel) continue;
		TPZSubCompMesh *submesh = dynamic_cast<TPZSubCompMesh *>(cel);
		if(!submesh) continue;
		submesh->MakeAllInternal();
	}
	fCMesh->CleanUpUnconnectedNodes();
#endif
}

    /// identify cornernodes
void TPZGenSubStruct::IdentifyCornerNodes()
{
#ifdef COMPLETE
  TPZNodesetCompute nodeset;
  TPZStack<int> elementgraph,elementgraphindex;
//    fCompMesh->ComputeElGraph(elementgraph,elementgraphindex);
  int nindep = fCMesh->NIndependentConnects();
//  int neq = fCMesh->NEquations();
  fCMesh->ComputeElGraph(elementgraph,elementgraphindex);
  int nel = elementgraphindex.NElements()-1;
  TPZMetis renum(nel,nindep);
    //nodeset.Print(file,elementgraphindex,elementgraph);
  renum.ConvertGraph(elementgraph,elementgraphindex,nodeset.Nodegraph(),nodeset.Nodegraphindex());
  //   cout << "nodegraphindex " << nodeset.Nodegraphindex() << endl;
  //   cout << "nodegraph " << nodeset.Nodegraph() << endl;
  nodeset.AnalyseGraph();
#ifdef LOG4CXX
  {
    std::stringstream str;
    nodeset.Print(str);
    LOGPZ_DEBUG(logger,str.str());
  }
#endif
  int nnodes = nodeset.Levels().NElements();
  int maxlev = nodeset.MaxLevel();
  int in;
  for(in=0; in<nnodes; in++)
  {
    if(nodeset.Levels()[in] == maxlev) 
    {
//      this->fCornerEqs.insert(in);
		int seqnum = fCMesh->ConnectVec()[in].SequenceNumber();
      int pos = fCMesh->Block().Position(seqnum);
      int size = fCMesh->Block().Size(seqnum);
      int ieq;
      for(ieq=0; ieq<size; ieq++)
      {
        this->fCornerEqs.insert(pos+ieq);
      }
    }
  }
  
#else
  fCMesh->ComputeNodElCon();
  int nindep = fCMesh->NIndependentConnects();
  int in;
  for(in=0; in<nindep; in++)
  {
    if(fCMesh->ConnectVec()[in].NElConnected() > 2 && fDimension == 2 ||
       fCMesh->ConnectVec()[in].NElConnected() > 4 && fDimension == 3 )
    {
		int seqnum = fCMesh->ConnectVec()[in].SequenceNumber();
      int pos = fCMesh->Block().Position(seqnum);
      int size = fCMesh->Block().Size(seqnum);
      int ieq;
      for(ieq=0; ieq<size; ieq++)
      {
        this->fCornerEqs.insert(pos+ieq);
      }
    }
  }
  
#endif
#ifdef LOG4CXX
  {
    std::stringstream str;
    str << "number of corner indices " << fCornerEqs.size() << std::endl;
    str << " corner connect indices ";
    std::set<int>::iterator it;
    for(it=fCornerEqs.begin(); it!=fCornerEqs.end(); it++)
    {
      str << *it << " ";
    }
    LOGPZ_DEBUG(logger,str.str());
  }
#endif
  
  
  
}

    /// initialize the TPZDohrMatrix structure
void TPZGenSubStruct::InitializeDohr(TPZAutoPointer<TPZMatrix> dohrmatrix, TPZAutoPointer<TPZDohrAssembly> assembly)
{
  //Isolate each subcompmesh and put it in the dohrmann matrix
  TPZDohrMatrix *dohr = dynamic_cast<TPZDohrMatrix *>(dohrmatrix.operator->());
  fCMesh->ComputeNodElCon();
	
  int neq = fCMesh->NEquations();
  dohr->Resize(neq,neq);
	// fCornerEqs was initialized during the mesh generation process
    dohr->SetNumCornerEqs(this->fCornerEqs.size());

  int nsub = NSubMesh(fCMesh);
	assembly->fFineEqs.Resize(nsub);
	assembly->fCoarseEqs.Resize(nsub);
	int isub;
  std::cout << "Computing the system of equations for each substructure\n";
  for(isub=0; isub<nsub; isub++)
  {
	  TPZSubCompMesh *submesh = SubMesh(fCMesh, isub);
    if(!submesh) 
    {
      continue;
    }
    std::cout << '*';
	  // creating the substructure HERE
    std::cout.flush();
    TPZAutoPointer<TPZDohrSubstruct> substruct = new TPZDohrSubstruct();
  // for each subcompmesh, reorder the nodes
    TPZAnalysis an(submesh);
    
    //keep the original sequence numbers of the connects
    
//    int nc = submesh->ConnectVec().NElements();
//    TPZManVector<int> origseqnum(nc);
//    int ic;
//    for(ic=0; ic<nc; ic++) origseqnum[ic] = submesh->ConnectVec()[ic].SequenceNumber();
    
  // compute the stiffness matrix
	  int neq = ((TPZCompMesh *)submesh)->NEquations();
//    int neq = substruct->fStiffness->Rows();
	  
	  substruct->fNEquations = neq;
    
    // identify the equation numbers of the submesh
    std::map<int,int> globinv;
	  // initialize the fGlobalIndex data structure
	  // fGlobalIndex will have -1 entries for internal equations
	  // we need to dismember this (again) in two vectors
	  IdentifyEqNumbers(submesh, substruct->fGlobalEqs,globinv);
	  int next = substruct->fGlobalEqs.NElements();
	  assembly->fFineEqs[isub].Resize(next);
	  int ieq;
	  for(ieq=0; ieq<next; ieq++)
	  {
		  assembly->fFineEqs[isub][ieq] = substruct->fGlobalEqs[ieq].second; 
	  }
	  
	  
    IdentifyEqNumbers(submesh, substruct->fGlobalIndex,globinv);
    
    // initialize the fC matrix
    // associate each column of the fC matrix with a coarse index
	  IdentifySubCornerEqs(globinv,substruct->fCoarseNodes,substruct->fCoarseIndex);
    substruct->fC.Redim(substruct->fCoarseNodes.NElements(),neq);
    for(ieq = 0; ieq<substruct->fCoarseNodes.NElements(); ieq++)
    {
      substruct->fC(ieq,substruct->fCoarseNodes[ieq]) = 1.;
    }
	  int ncoarse = substruct->fCoarseIndex.NElements();
	  assembly->fCoarseEqs[isub].Resize(ncoarse);
	  for(ieq=0; ieq<ncoarse; ieq++)
	  {
		  assembly->fCoarseEqs[isub][ieq] = substruct->fCoarseIndex[ieq];
	  }
    
  // reorder by internal nodes
	  // the fInternalEqs data structure will not be filled if the connects are made internal
	  
	  // this permutes the nodes of the submesh
	  // This is a lengthy process which should run on the remote processor
	InitializeMatrices(submesh, substruct,  assembly);

#ifdef LOG4CXX
    {
      std::stringstream sout;
/*      sout << "Submesh for element " << iel << std::endl;
      submesh->Print(sout);*/
      sout << "Substructure for submesh " << isub << std::endl;
      substruct->Print(sout);
      LOGPZ_DEBUG(logger,sout.str())
    }
#endif
    dohr->AddSubstruct(substruct);
  }
  std::cout << std::endl;
}

/// identify the global equations as a pair of local equation and global equation
void TPZGenSubStruct::IdentifyEqNumbers(TPZSubCompMesh *sub, TPZVec<std::pair<int,int> > &globaleq, std::map<int,int> &globinv)
{
	int ncon = sub->ConnectVec().NElements();
	// ncon is the number of connects of the subcompmesh
	TPZCompMesh *subcomp = (TPZCompMesh *) sub;
	globaleq.Resize(subcomp->NEquations(),std::pair<int,int>(-1,-1));
	TPZCompMesh *super = fCMesh.operator->();
	int count = 0;
	int ic;
#ifdef LOG4CXX_STOP
	std::stringstream sout;
	sout << "total submesh connects/glob/loc ";
#endif
	for(ic=0; ic<ncon; ic++)
	{
		int glob = sub->NodeIndex(ic,super);
		// continue is the connect is internal
		if(glob == -1) continue;
		int locseq = sub->ConnectVec()[ic].SequenceNumber();
		int globseq = super->ConnectVec()[glob].SequenceNumber();
		int locpos = sub->Block().Position(locseq);
		int globpos = super->Block().Position(globseq);
		int locsize = sub->Block().Size(locseq);
		//    int globsize = super->Block().Size(globseq);
		int ieq;
		for(ieq =0; ieq<locsize; ieq++)
		{
#ifdef LOG4CXX_STOP
			sout << ic << "/" << globpos+ieq << "/" << locpos+ieq << " ";
#endif
			globaleq[count] = std::pair<int,int>(locpos+ieq,globpos+ieq);
			count++;
			globinv[globpos+ieq] = locpos+ieq;
		}
	}
	globaleq.Resize(count);
#ifdef LOG4CXX_STOP
	LOGPZ_DEBUG(logger,sout.str())
#endif
}


    /// get the global equation numbers of a substructure (and their inverse)
void TPZGenSubStruct::IdentifyEqNumbers(TPZSubCompMesh *sub, TPZVec<int> &global, std::map<int,int> &globinv)
{
  int ncon = sub->ConnectVec().NElements();
	// ncon is the number of connects of the subcompmesh
  TPZCompMesh *subcomp = (TPZCompMesh *) sub;
  global.Resize(subcomp->NEquations(),-1);
  TPZCompMesh *super = fCMesh.operator->();
  int ic;
#ifdef LOG4CXX_STOP
  std::stringstream sout;
  sout << "total submesh connects/glob/loc ";
#endif
  for(ic=0; ic<ncon; ic++)
  {
    int glob = sub->NodeIndex(ic,super);
	  // continue is the connect is internal
    if(glob == -1) continue;
    int locseq = sub->ConnectVec()[ic].SequenceNumber();
    int globseq = super->ConnectVec()[glob].SequenceNumber();
    int locpos = sub->Block().Position(locseq);
    int globpos = super->Block().Position(globseq);
    int locsize = sub->Block().Size(locseq);
//    int globsize = super->Block().Size(globseq);
    int ieq;
    for(ieq =0; ieq<locsize; ieq++)
    {
#ifdef LOG4CXX_STOP
      sout << ic << "/" << globpos+ieq << "/" << locpos+ieq << " ";
#endif
      global[locpos+ieq] = globpos+ieq;
      globinv[globpos+ieq] = locpos+ieq;
    }
  }
#ifdef LOG4CXX_STOP
  LOGPZ_DEBUG(logger,sout.str())
#endif
}


/*!
    \fn TPZGenSubStruct::ReorderInternalNodes(TPZSubCompMesh *sub)
 */
void TPZGenSubStruct::ReorderInternalNodes(TPZSubCompMesh *sub,std::map<int,int> &globaltolocal, TPZVec<int> &internalnodes)
{
	TPZVec<int> permute;
  sub->PermuteInternalFirst(permute);
  int ninternal = this->NInternalEq(sub);
  internalnodes.Resize(ninternal);
	// this datastructure will not be initialized if the connects are made internal
	internalnodes.Fill(-1);
  int ncon = sub->ConnectVec().NElements();
  TPZCompMesh *super = fCMesh.operator->();
#ifdef LOG4CXX_STOP
  std::stringstream sout;
  sout << "internal submesh connects/glob/loc ";
#endif
      int ic;
  for(ic=0; ic<ncon; ic++)
  {
    int glob = sub->NodeIndex(ic,super);
    if(glob == -1) continue;
    int locseq = sub->ConnectVec()[ic].SequenceNumber();
    int globseq = super->ConnectVec()[glob].SequenceNumber();
    int locpos = sub->Block().Position(locseq);
    int globpos = super->Block().Position(globseq);
    int locsize = sub->Block().Size(locseq);
//    int globsize = super->Block().Size(globseq);
    int ieq;
    for(ieq =0; ieq<locsize; ieq++)
    {
      if(locpos+ieq < ninternal)
      {
#ifdef LOG4CXX_STOP
      sout << ic << "/" << globpos+ieq << "/" << locpos+ieq << " ";
#endif
        internalnodes[locpos+ieq] = globaltolocal[globpos+ieq];
      }
    }
  }
#ifdef LOG4CXX_STOP
  LOGPZ_DEBUG(logger,sout.str())
#endif
  
}

/*!
 \fn TPZGenSubStruct::ReorderInternalNodes(TPZSubCompMesh *sub)
 */
void TPZGenSubStruct::ReorderInternalNodes2(TPZSubCompMesh *sub, TPZVec<int> &internalnodes, TPZVec<int> &invpermute)
{
	TPZBlock prevblock = sub->Block();
	TPZVec<int> permute;
	sub->PermuteInternalFirst(permute);
	invpermute.Resize(permute.NElements());
	int i;
	for(i=0; i<permute.NElements(); i++)
	{
		invpermute[permute[i]] = i;
	}
	int ninternal = NInternalEq(sub);
	internalnodes.Resize(ninternal);
	// this datastructure will not be initialized if the connects are made internal
	internalnodes.Fill(-1);
	int ncon = sub->ConnectVec().NElements();
#ifdef LOG4CXX_STOP
	std::stringstream sout;
	sout << "internal submesh connects/glob/loc ";
#endif
	int ic;
	for(ic=0; ic<ncon; ic++)
	{
		int locseq = sub->ConnectVec()[ic].SequenceNumber();
		int origseq = invpermute[locseq];
		int locpos = sub->Block().Position(locseq);
		int origpos = prevblock.Position(origseq);
		int size = sub->Block().Size(locseq);
		//    int globsize = super->Block().Size(globseq);
		int ieq;
		for(ieq =0; ieq<size; ieq++)
		{
			if(locpos+ieq < ninternal)
			{
#ifdef LOG4CXX_STOP
				sout << ic << "/" << globpos+ieq << "/" << locpos+ieq << " ";
#endif
				internalnodes[locpos+ieq] = origpos+ieq;
			}
		}
	}
#ifdef LOG4CXX_STOP
	LOGPZ_DEBUG(logger,sout.str())
#endif
	
}

/// Identify the corner equations associated with a substructure
void TPZGenSubStruct::IdentifySubCornerEqs(std::map<int,int> &globaltolocal, TPZVec<int> &cornereqs,
                                          TPZVec<int> &coarseindex)
{
#ifdef LOG4CXX
	{
		std::stringstream sout;
		sout << "Input data for IdentifySubCornerEqs \nglobaltolocal";
		std::map<int,int>::iterator mapit;
		for(mapit = globaltolocal.begin(); mapit != globaltolocal.end(); mapit++)
		{
			sout << " [" << mapit->first << " , " << mapit->second << "] ";
		}
		sout << "\nCorner equations stored in the GenSubStructure data ";
		std::set<int>::iterator setit;
		for(setit = fCornerEqs.begin(); setit != fCornerEqs.end(); setit++)
		{
			sout << *setit << " , ";
		}
		sout << "\ncornereqs " << cornereqs;
		LOGPZ_DEBUG(logger,sout.str())
	}
#endif
  std::set<int>::iterator it;
  std::list<int> subcorn,coarseindexlist;
  int count = 0;
  for(it = fCornerEqs.begin(); it!= fCornerEqs.end(); it++,count++)
  {
    if(globaltolocal.find(*it) != globaltolocal.end())
    {
      subcorn.push_back(globaltolocal[*it]);
      coarseindexlist.push_back(count);
    }
  }
  std::list<int>::iterator lit;
  cornereqs.Resize(subcorn.size());
  coarseindex.Resize(coarseindexlist.size());
  count = 0;
  for(lit=subcorn.begin(); lit != subcorn.end(); lit++)
  {
    cornereqs[count++] = *lit;
  }
  count = 0;
  for(lit = coarseindexlist.begin(); lit != coarseindexlist.end(); lit++)
  {
    coarseindex[count++] = *lit;
  }
}


/*!
    \fn TPZGenSubStruct::NInternalEq(TPZSubCompMesh *sub)
 */
int TPZGenSubStruct::NInternalEq(TPZSubCompMesh *sub)
{
  std::list<int> internal;
  sub->PotentialInternal(internal);
  std::list<int>::iterator it;
  int result = 0;
  for(it=internal.begin(); it != internal.end(); it++)
  {
    int seq = sub->ConnectVec()[*it].SequenceNumber();
    int sz = sub->Block().Size(seq);
    result += sz;
  }
  return result;
}

// This is a lengthy process which should run on the remote processor
void InitializeMatrices(TPZSubCompMesh *submesh, TPZAutoPointer<TPZDohrSubstruct> substruct, TPZDohrAssembly &dohrassembly)
{
	// this should happen in the remote processor
    TPZSkylineStructMatrix skylstr(submesh);
    substruct->fStiffness = TPZAutoPointer<TPZMatrix> (skylstr.CreateAssemble(substruct->fLocalLoad));
	
	// This should happen in the remote processor
    substruct->fInvertedStiffness.SetMatrix(substruct->fStiffness->Clone());
    substruct->fInvertedStiffness.SetDirect(ECholesky);
	
	TPZManVector<int> invpermute;
	TPZGenSubStruct::ReorderInternalNodes2(submesh,substruct->fInternalEqs,invpermute);
	// compute the stiffness matrix associated with the internal nodes
	// fInternalEqs indicates the permutation of the global equations to the numbering of the internal equations
	// this is meaningless if the internal nodes are invisible to the global structure
    int ninternal = substruct->fInternalEqs.NElements();
	// THIS SHOULD HAPPEN IN THE REMOTE PROCESSOR
    skylstr.SetEquationRange(0,ninternal);
    TPZFMatrix rhs;
    substruct->fInvertedInternalStiffness.SetMatrix(skylstr.CreateAssemble(rhs));
    substruct->fInvertedInternalStiffness.SetDirect(ECholesky);
	substruct->ComputeCoarseStiffness();

    // put back the original sequence numbers of the connects (otherwise we can't apply a load solution
    
	submesh->Permute(invpermute);
	
}

/// return the number of submeshes
int NSubMesh(TPZAutoPointer<TPZCompMesh> compmesh)
{
	int nel = compmesh->NElements();
	TPZCompEl *cel;
	int iel, count = 0;
	for(iel=0; iel<nel; iel++)
	{
		cel = compmesh->ElementVec()[iel];
		if(!cel) continue;
		TPZSubCompMesh *sub = dynamic_cast<TPZSubCompMesh *>(cel);
		if(sub) count++;
	}
	return count;
}

/// return a pointer to the isub submesh
TPZSubCompMesh *SubMesh(TPZAutoPointer<TPZCompMesh> compmesh, int isub)
{
	int nel = compmesh->NElements();
	TPZCompEl *cel;
	int iel, count = 0;
	for(iel=0; iel<nel; iel++)
	{
		cel = compmesh->ElementVec()[iel];
		if(!cel) continue;
		TPZSubCompMesh *sub = dynamic_cast<TPZSubCompMesh *>(cel);
		if(sub && isub == count) return sub;
		if(sub) count++;
	}
	return NULL;
}
