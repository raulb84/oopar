/* Generated by Together */

#ifndef TBICGSTAB_H
#define TBICGSTAB_H
#include <ooptask.h>
#include <TLoopFor.h>
#include <TDistNorm.h>


class TBiCGStab : public OOPTask {
public:    

  TBiCGStab(int nproc);
  ~TBiCGStab();
  /**
  * Returns the estimated execution time.
  * returns 0 if the task is instantaneous
  * returns > 0 if estimate is known
  * return < 0 if no estimate is known
  */
	virtual long ExecTime ();

	/**
	* Execute the task, verifying that all needed data acesses are satisfied.
	*/
	virtual OOPMReturnType Execute ();

	/**
	* Returns last created Id.
	*/
	virtual long GetClassID ()
	{
		return TDISTNORM_ID;
	}
  
private:
	/**
	 * Submits the necessary data objects for the BiCGStab completion
	 */
	void SubmitObjects();
	/**
	 * Sets up task/data dependency relationship.
	 */
	void SetupTaskData();
  /** This function declares and submits the loop found on the BiCGStab code.
@since 09-01-2004
@author Gustavo C. Longhin
 */
    void CreateLoop(OOPDataVersion & normbVersion);

	/**
	 * Global Ids
	 */
	OOPObjectId fId_normb;
	OOPObjectId fId_normr;
	OOPObjectId fId_resid;
	OOPObjectId fId_rho_1;
	OOPObjectId fId_rho_2;
	OOPObjectId fId_alpha;
	OOPObjectId fId_beta;
	OOPObjectId fId_omega;
	OOPObjectId fId_max_iter;
	OOPObjectId fId_tol;

	/**
	 * Local Ids
	 */
	vector <OOPObjectId> f_lId_A;
	vector <OOPObjectId> f_lId_M;
	vector <OOPObjectId> f_lId_x;
	vector <OOPObjectId> f_lId_b;
	vector <OOPObjectId> f_lId_p;
	vector <OOPObjectId> f_lId_shat;
	vector <OOPObjectId> f_lId_s;
	vector <OOPObjectId> f_lId_phat;
	vector <OOPObjectId> f_lId_t;
	vector <OOPObjectId> f_lId_v;
	vector <OOPObjectId> f_lId_CMatrix;
	vector <OOPObjectId> f_lId_rtilde;
	vector <OOPObjectId> f_lId_r;



};
#endif //TBICGSTAB_H