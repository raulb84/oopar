/* Generated by Together */

#include "TLoopFor.h"
#include "TDotProduct.h"
#include <tbicgfor_one.h>
#include <oopmetadata.h>
#include <ooptaskmanager.h>
#include <oopcommmanager.h>

void TLoopFor::SubmitPartOne(){
/*
    if (rho_1 == 0) {
      tol = Norm(r) / normb;
      return 2;
    }
    if (i == 1)
      p = r;
    else {
      //beta(0) = (rho_1(0)/rho_2(0)) * (alpha(0)/omega(0));
      beta = (rho_1/rho_2) * (alpha/omega); //TComputeBeta
      //p = r + beta(0) * (p - omega(0) * v);
      p *= beta; //p.Add(beta,1)
      p.Add(1., r);
      p.Add(- beta * omega, v);
    }
    M.Solve(p, phat);//parei aqui.
*/

 OOPDataVersion rho_version = fDataDepend.Dep(fId_rho_1).ObjPtr()->Version();
 rho_version.Increment();

 //First if necessary data
 OOPDataVersion tolVersion = fDataDepend.Dep(fId_tol).ObjPtr()->Version();
 OOPDataVersion normr_ver = fDataDepend.Dep(fId_normr).ObjPtr()->Version();
 OOPDataVersion rver = fDataDepend.Dep(f_lId_r[0]).ObjPtr()->Version();
 OOPDataVersion normb_ver = fDataDepend.Dep(fId_normb).ObjPtr()->Version();


 //Sencond If necessary data
 //Condition
 OOPDataVersion max_iter_Version = fDataDepend.Dep(fId_max_iter).ObjPtr()->Version();
 //True operation
 OOPDataVersion p_Version = fDataDepend.Dep(f_lId_p[0]).ObjPtr()->Version();
 OOPDataVersion r_Version = fDataDepend.Dep(f_lId_r[0]).ObjPtr()->Version();
 
 //False operation
 OOPDataVersion beta_ver = fDataDepend.Dep(fId_beta).ObjPtr()->Version();
 //rho_1 already dependent
 OOPDataVersion rho_2_ver = fDataDepend.Dep(fId_rho_2).ObjPtr()->Version();
 OOPDataVersion alpha_Version = fDataDepend.Dep(fId_alpha).ObjPtr()->Version();
 OOPDataVersion omega_Version = fDataDepend.Dep(fId_omega).ObjPtr()->Version();
 //p already dependent
 //r already dependent
 OOPDataVersion v_Version = fDataDepend.Dep(f_lId_v[0]).ObjPtr()->Version();

 //M.Solve(p,phat)
 OOPDataVersion M_Version = fDataDepend.Dep(f_lId_M[0]).ObjPtr()->Version();
 OOPDataVersion phat_Version = fDataDepend.Dep(f_lId_phat[0]).ObjPtr()->Version();


 int nproc = CM->NumProcessors();
 TBiCGFor_One * bicgforone;
 int i = 0;
 for(i=0;i<nproc;i++){
       bicgforone = new TBiCGFor_One(i);
       bicgforone->AddDependentData(OOPMDataDepend(fId_rho_1,EReadAccess,rho_version));
       bicgforone->AddDependentData(OOPMDataDepend(fId_tol,EReadAccess,tolVersion));
       bicgforone->AddDependentData(OOPMDataDepend(fId_normr,EReadAccess,normr_ver));
       bicgforone->AddDependentData(OOPMDataDepend(f_lId_r[i],EReadAccess,r_Version));
       bicgforone->AddDependentData(OOPMDataDepend(fId_normb,EReadAccess,normb_ver));
       bicgforone->AddDependentData(OOPMDataDepend(fId_max_iter, EReadAccess, max_iter_Version));
       bicgforone->AddDependentData(OOPMDataDepend(f_lId_p[i], EWriteAccess, p_Version));
       bicgforone->AddDependentData(OOPMDataDepend(f_lId_r[i], EReadAccess, r_Version));
       bicgforone->AddDependentData(OOPMDataDepend(fId_beta, EWriteAccess, beta_ver));
       bicgforone->AddDependentData(OOPMDataDepend(fId_rho_2, EReadAccess, rho_2_ver));
       bicgforone->AddDependentData(OOPMDataDepend(fId_alpha, EReadAccess, alpha_Version));
       bicgforone->AddDependentData(OOPMDataDepend(fId_omega, EReadAccess, omega_Version));
       bicgforone->AddDependentData(OOPMDataDepend(f_lId_v[i], EReadAccess, v_Version));
       bicgforone->AddDependentData(OOPMDataDepend(f_lId_M[i], EReadAccess, M_Version));
       bicgforone->AddDependentData(OOPMDataDepend(f_lId_phat[i], EReadAccess, phat_Version));
 }
 

 


 

 
 
 

 
 /*
 TIfConditional * ifcond = new TIfConditional(0);
 ifcond->AddDependentData(OOPMDataDepend(fId_rho_1, ERead, version));
 ifcond->AddDependentData(OOPMDataDepend(fId_tol, EWrite, tolVersion));
 ifcond->AddDependentData(OOPMDataDepend(fId_normr, ERead, tolVersion));
 ifcond->AddDependentData(OOPMDataDepend(fId_normb, ERead, tolVersion));
 ifcond->Submit();
 
  * fId_tol will be incremented anyway -->n+1
  */
}
TLoopFor::TLoopFor(int proc) : OOPTask(proc) {}

OOPMReturnType TLoopFor::Execute ()
{
  /**
   * Submit the dot product between rtilde and r
   */
  SubmitDistDotProduct();

  /**
   * Submit the if conditional.
   * fId_tol incremented.
   */
  SubmitPartOne();


  return ESuccess;
}

void TLoopFor::SubmitDistDotProduct()
{
  int i, nproc = CM->NumProcessors();
  TDotProduct * dotprod;
  /**
   * Setting the correct version for the rho_1 object
   */
  OOPDataVersion version = fDataDepend.Dep(fId_rho_1).ObjPtr()->Version();
  version.IncrementLevel(nproc);
  fDataDepend.Dep(fId_rho_1).ObjPtr()->SetVersion(version, Id());
  /**
   * Setting the level version for the dependency relation according to dotprod
   */
  version.SetLevelVersion(3,-1);
  for(i=0;i<nproc;i++){
    dotprod = new TDotProduct(i);
    dotprod->AddDependentData(OOPMDataDepend(fId_rho_1, EWriteAccess, version));
    OOPDataVersion rtildeVersion = fDataDepend.Dep(f_lId_rtilde[i]).ObjPtr()->Version();
    dotprod->AddDependentData(OOPMDataDepend(f_lId_rtilde[i],EReadAccess,rtildeVersion));
    OOPDataVersion rVersion = fDataDepend.Dep(f_lId_r[i]).ObjPtr()->Version();
    dotprod->AddDependentData(OOPMDataDepend(f_lId_r[i],EReadAccess,rVersion));
    dotprod->Submit();
  }
}

long TLoopFor::ExecTime ()
{
	return -1;
}
void TLoopFor::SetupVersions(){
  int i;
	OOPDataVersion auxver;

	auxver = fDataDepend.Dep(fId_max_iter).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_max_iter).ObjPtr()->SetVersion(auxver, Id());


	auxver = fDataDepend.Dep(fId_rho_1).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_rho_1).ObjPtr()->SetVersion(auxver, Id());


	auxver = fDataDepend.Dep(fId_rho_2).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_rho_2).ObjPtr()->SetVersion(auxver, Id());

  auxver = fDataDepend.Dep(fId_alpha).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_alpha).ObjPtr()->SetVersion(auxver, Id());

	auxver = fDataDepend.Dep(fId_omega).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_omega).ObjPtr()->SetVersion(auxver, Id());

	auxver = fDataDepend.Dep(fId_beta).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_beta).ObjPtr()->SetVersion(auxver, Id());

	auxver = fDataDepend.Dep(fId_normb).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_normb).ObjPtr()->SetVersion(auxver,Id());

	auxver = fDataDepend.Dep(fId_tol).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_tol).ObjPtr()->SetVersion(auxver, Id());

  auxver = fDataDepend.Dep(fId_normr).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_normr).ObjPtr()->SetVersion(auxver, Id());

	auxver = fDataDepend.Dep(fId_resid).ObjPtr()->Version();
	auxver.IncrementLevel(-1);
	fDataDepend.Dep(fId_resid).ObjPtr()->SetVersion(auxver, Id());

  int nproc = CM->NumProcessors();
  for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_p[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_p[i]).ObjPtr()->SetVersion(auxver, Id());
	}

  for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_A[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_A[i]).ObjPtr()->SetVersion(auxver, Id());
	}

	for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_M[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_M[i]).ObjPtr()->SetVersion(auxver, Id());
	}

	for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_v[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_v[i]).ObjPtr()->SetVersion(auxver, Id());
	}

	for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_phat[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_phat[i]).ObjPtr()->SetVersion(auxver, Id());
	}

	for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_s[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_s[i]).ObjPtr()->SetVersion(auxver, Id());
	}

	for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_x[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_x[i]).ObjPtr()->SetVersion(auxver, Id());
	}

	for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_shat[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_shat[i]).ObjPtr()->SetVersion(auxver, Id());
	}

	for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_t[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_t[i]).ObjPtr()->SetVersion(auxver, Id());
	}

	for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_rtilde[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_rtilde[i]).ObjPtr()->SetVersion(auxver, Id());
	}

	for(i=0;i<nproc;i++){
		auxver = fDataDepend.Dep(f_lId_r[i]).ObjPtr()->Version();
  	auxver.IncrementLevel(-1);
  	fDataDepend.Dep(f_lId_r[i]).ObjPtr()->SetVersion(auxver, Id());
	}
	
}
