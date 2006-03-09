#ifdef OOP_MPI
#include "oopmpicomm.h"
#endif
#include "ooptaskmanager.h"
#include "oopcommmanager.h"
#include "oopdatamanager.h"
#include "ooptaskcontrol.h"

#include "oopwaittask.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sstream>
#include <iostream>

#include <sys/time.h>
class OOPStorageBuffer;
class   OOPTask;
class   OOPDataVersion;
class   OOPSaveable;
class   OOPObjectId;
class	OOPTerminationTask;
//#include <sys/types.h>
//#include <unistd.h>

#include <sstream>
#include <pzlog.h>
#ifdef LOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
using namespace log4cxx;
using namespace log4cxx::helpers;
static LoggerPtr logger(Logger::getLogger("OOPAR.OOPTaskManager"));
static LoggerPtr tasklogger(Logger::getLogger("OOPAR.OOPTaskManager.task"));
#endif

void OOPTaskManager::main ()
{
}

void OOPTaskManager::SetNumberOfThreads(const int n){
  OOPTaskManager::fNumberOfThreads = n;
}

int OOPTaskManager::NumberOfThreads(){
  return OOPTaskManager::fNumberOfThreads;
}
        
void OOPTaskManager::TransferExecutingTasks(){
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_ERROR (logger,sout.str());
    
#endif
    return;
  }
  list < OOPTaskControl * >::iterator sub;
  //int listsize = fExecuting.size();
  sub = fExecuting.begin();
  OOPTaskControl * auxtc=0;
  while (sub != fExecuting.end())
  {
    bool isfinished=false;
    auxtc = (*sub);
    //pthread_mutex_lock(&fExecutingMutex);
    if(auxtc)
    {
      isfinished = auxtc->TaskFinished();
    } else
    {
#ifdef LOGPZ      
      stringstream sout;
      sout << __FUNCTION__ << " I dont understand \n";
      LOGPZ_ERROR (logger,sout.str());
#endif      
    }
  
    //pthread_mutex_unlock(&fExecutingMutex);
    if (isfinished){
      auxtc->Join();
#ifdef LOGPZ        
      stringstream sout;
      sout << __PRETTY_FUNCTION__ << "Task finished " << auxtc->Task()->Id() << " classid " << auxtc->Task()->ClassId();
      LOGPZ_DEBUG (logger,sout.str());
#endif      
      OOPObjectId id;
      id = auxtc->Task ()->Id ();
      auxtc->Depend ().SetExecuting (id, false);
      // this method may want to grab the mutex!!!
      auxtc->Depend ().ReleaseAccessRequests (id);
      auxtc->Task()->Depend().ClearPointers();
      fFinished.push_back(auxtc);
      list < OOPTaskControl * >::iterator keep;
      keep = sub;
      fExecuting.erase(keep);
      // a finished task may have sneeked in during the unlock/lock sequence
      sub = fExecuting.begin();
    } else
    {
      sub++;
    }
  }

}

#ifdef OOP_MPI
//#define MT

/*
void * OOPTaskManager::TriggerTask(void * data){
	OOPTaskControl * tc = static_cast<OOPTaskControl *> (data);
	OOPTaskManager * lTM = dynamic_cast<OOPTaskManager *> (TM);
	tc->Task ()->Execute ();
	pthread_mutex_lock(&lTM->fExecutingMutex);
	tc->Task()->SetExecuting(false);
	pthread_mutex_unlock(&lTM->fExecutingMutex);
	return NULL;
}
*/

/**
	disparar o thread de execu�o da tarefa.
*/
void * OOPTaskManager::ExecuteMT(void * data){
  OOPTaskManager * lTM = static_cast<OOPTaskManager *>(data);
  //Qual �o service thread ?
  // O service thread e a linha de execucao do programa principal
  
  pthread_mutex_lock(&lTM->fSubmittedMutex);
  DM->SubmitAllObjects();
  CM->ReceiveMessages ();
  // this method needs to grab the lock
  lTM->TransferSubmittedTasks ();
  list < OOPTaskControl * >::iterator i;
#ifdef LOGPZ    
  {
    std::stringstream sout;
    sout << "Entering task list loop";
    LOGPZ_DEBUG (logger,sout.str());
  }
#endif  
  lTM->fKeepGoing=true;
  lTM->ExecuteDaemons();
  while (lTM->fKeepGoing) {
    CM->ReceiveMessages();
    lTM->ExecuteDaemons();
    while ( lTM->fExecutable.size () && (int)lTM->fExecuting.size() < lTM->fNumberOfThreads )
    {
      i = lTM->fExecutable.begin ();
      OOPTaskControl *tc = (*i);
      lTM->fExecutable.erase(i);
      lTM->fExecuting.push_back(tc);
      tc->Task()->SetExecuting(true);
#ifdef LOGPZ        
      {
        stringstream sout;
        sout << "Entering taskcontrol execute for task " << tc->Task()->Id() << " classid " << tc->Task()->ClassId();
        LOGPZ_DEBUG (tasklogger,sout.str());
      }
#endif      
      tc->Execute();
      lTM->TransferExecutingTasks();
      DM->SubmitAllObjects();
    }
    lTM->TransferExecutingTasks();
    lTM->TransferFinishedTasks ();
    //    if(!CM->GetProcID()) cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << " before receive " << CM->GetProcID() <<   "\n";
    CM->ReceiveMessages ();
    //    if(!CM->GetProcID()) cout << __PRETTY_FUNCTION__ << ":" << __LINE__ << " after receive " << CM->GetProcID() <<   "\n";
    
    // give a chance to waiting threads
    pthread_mutex_unlock(&lTM->fSubmittedMutex);
    pthread_mutex_lock(&lTM->fSubmittedMutex);
    lTM->TransferSubmittedTasks ();
    CM->SendMessages ();
    lTM->ExecuteDaemons();
    //wait
    if(!lTM->HasWorkTodo () && lTM->fKeepGoing)
    {
      timeval now;
      gettimeofday(&now,0);
      now.tv_usec += 1;
      now.tv_sec += now.tv_usec/1000000;
      now.tv_usec %= 1000000;
      timespec next;
      next.tv_sec = now.tv_sec;
      next.tv_nsec = now.tv_usec*1000;
      //        cout << __PRETTY_FUNCTION__ << " TaskManager going to sleep\n";
      cout.flush();
      if(CM->NumProcessors() > 1)
      {
        pthread_cond_timedwait(&lTM->fExecuteCondition, &lTM->fSubmittedMutex,&next);
      } else
      {
        pthread_cond_wait(&lTM->fExecuteCondition, &lTM->fSubmittedMutex);
      }
    }
  }
  CM->SendMessages ();
  pthread_mutex_unlock(&lTM->fSubmittedMutex);
  return NULL;
}
#endif
OOPTaskManager::OOPTaskManager (int proc) : fNumberOfThreads(2),
  fLockThread(0), fLock(0)
{
	fProc = proc;
	fLastCreated = 0;//NUMOBJECTS * fProc;
//	fMaxId = fLastCreated + NUMOBJECTS;

//	pthread_mutex_init(&fExecutingMutex, NULL);
//	pthread_mutex_init(&fFinishedMutex, NULL);
	pthread_mutex_init(&fSubmittedMutex, NULL);
  pthread_cond_init(&fExecuteCondition, NULL);
}
OOPTaskManager::~OOPTaskManager ()
{
	list < OOPTask * >::iterator i;
	for (i = fSubmittedList.begin (); i != fSubmittedList.end (); i++)
		delete *i;
	list < OOPTaskControl * >::iterator itc;
	for (itc = fExecutable.begin (); itc != fExecutable.end (); itc++)
		delete *itc;
	for (itc = fFinished.begin (); itc != fFinished.end (); itc++)
		delete *itc;
	for (itc = fTaskList.begin (); itc != fTaskList.end (); itc++)
		delete *itc;
}
//Passar o ID do task e do dado.
void OOPTaskManager::NotifyAccessGranted (const OOPObjectId & TaskId,
					  const OOPMDataDepend & depend,
					  OOPMetaData * objptr)
{
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_ERROR (logger,sout.str());
#endif    
  }
	list < OOPTaskControl * >::iterator i;
	bool found = false;
	for (i = fTaskList.begin (); i != fTaskList.end (); i++) {
		OOPTaskControl *tc = (*i);
		if (tc->Task ()->Id () == TaskId) {
			found = true;
			tc->Depend ().GrantAccess (depend, objptr);
      {
#ifdef LOGPZ        
        stringstream sout;
        sout << "Access Granted to taskId " << TaskId  << " classid " << tc->Task()->ClassId();
        sout << " on data " << depend.Id();
        LOGPZ_DEBUG (tasklogger,sout.str());
#endif
      }
			if (tc->Depend ().CanExecute ()) {
				TransfertoExecutable (tc->Task ()->Id ());
        {
#ifdef LOGPZ          
          stringstream sout;
          sout << "OOPTaskManager task is executable " << TaskId << " classid " << tc->Task()->ClassId();
          LOGPZ_DEBUG (tasklogger,sout.str());
#endif          
        }
			}
			break;
		}
	}
	if (!found) {
#ifdef LOGPZ    
    stringstream sout;
		sout << "Task not found on current TM: File:" << __FILE__ <<
			" Line:" << __LINE__ << endl;
		sout << " Task ";
		TaskId.Print (sout);
    LOGPZ_ERROR (tasklogger,sout.str());
#endif    
	}
}
void OOPTaskManager::RevokeAccess (const OOPObjectId & TaskId,
				   const OOPMDataDepend & depend)
{
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_ERROR (logger,sout.str());
#endif    
  }
	list < OOPTaskControl * >::iterator i;
	bool found = false;
	for (i = fTaskList.begin (); i != fTaskList.end (); i++) {
		OOPTaskControl *tc = (*i);
		if (tc->Task ()->Id () == TaskId) {
			found = true;
			tc->Depend ().RevokeAccess (depend);
#ifdef LOGPZ        
      stringstream sout;
			sout << "Access Revoked to taskId " << TaskId << " classid " << tc->Task()->ClassId();
			sout << " on data " << depend.Id();
      LOGPZ_DEBUG (logger,sout.str());
#endif      
			break;
		}
	}
	if (!found) {
#ifdef LOGPZ    
    stringstream sout;
		sout << "OOPTaskManager::RevokeAccess Task not found on current TM: File:" << __FILE__ << " Line:" << __LINE__ << endl;
		sout << "Task ";
		TaskId.Print (sout);
    LOGPZ_DEBUG (logger,sout.str());
#endif    
	}
}
void OOPTaskManager::SubmitDaemon (OOPDaemonTask * task) {
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_DEBUG (logger,sout.str());
#endif    
  }
  if(task->GetProcID() != this->fProc) 
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " Sending a daemon task to proc " << task->GetProcID() << " classid " << task->ClassId();
#endif    
    CM->SendTask(task);
#ifdef LOGPZ      
    LOGPZ_DEBUG (logger,sout.str());
#endif    
    
  }
  else
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " Submitting a daemon task " << " classid " << task->ClassId();
    LOGPZ_DEBUG (logger,sout.str());
#endif    
    fDaemon.push_back(task);
  }
}
OOPObjectId OOPTaskManager::Submit (OOPTask * task)
{
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << "Calling Submit on OOPTaskManager ";
    LOGPZ_DEBUG (logger,sout.str());
#endif    
  }

	OOPDaemonTask *dmt = dynamic_cast < OOPDaemonTask * >(task);
        OOPObjectId id;
	if(dmt) {
#ifdef LOGPZ    
    stringstream sout;
    sout << "Task Submitted is a daemon";
    LOGPZ_DEBUG (logger,sout.str());
#endif    
	} else 
  {
    id = task->Id();
    if(id.IsZeroOOP()) id = GenerateId ();
    task->SetTaskId (id);
  }
  {
#ifdef LOGPZ    
    stringstream sout;     
    sout << __PRETTY_FUNCTION__ << " Task with id " << task->Id() << " submitted for processor" << task->GetProcID() << " classid " << task->ClassId();
    LOGPZ_DEBUG (tasklogger,sout.str());
#endif    
  }
#ifdef DEBUG
      OOPWaitTask *wait = dynamic_cast<OOPWaitTask *> (task);
  if(!wait && !dmt && !CM->GetProcID())
  {
      std::ostringstream FileName, FileName2,command,subdir1,subdir2,subdir3;
      subdir1 << "taskman" << CM->GetProcID();
      subdir2 << "taskman" << CM->GetProcID() << "/orig";
      subdir3 << "taskman" << CM->GetProcID() << "/copy";
      mkdir(subdir1.str().c_str() , S_IRWXU | S_IXGRP | S_IRGRP| S_IXOTH | S_IROTH );
      mkdir(subdir2.str().c_str(), S_IRWXU | S_IXGRP | S_IRGRP| S_IXOTH | S_IROTH );
      mkdir(subdir3.str().c_str(), S_IRWXU | S_IXGRP | S_IRGRP| S_IXOTH | S_IROTH );
      FileName << subdir2.str() << "/" << task->ClassId() << ".sav";
      FileName2 << subdir3.str() << "/" << task->ClassId() << ".sav";
     {
      TPZFileStream PZFS;
      PZFS.OpenWrite(FileName.str());
      task->Write(PZFS,1);
    }
     {
      TPZFileStream PZFS;
      PZFS.OpenRead(FileName.str());
      OOPTask *test = (OOPTask *) TPZSaveable::Restore(PZFS,0);
      TPZFileStream PZFS2;
      PZFS2.OpenWrite(FileName2.str());
      test->Write(PZFS2,1);
      delete test;
    }
    command << "diff --brief " << FileName.str() << " " << FileName2.str() << endl;
    FILE *pipe = popen(command.str().c_str(),"r");
#ifdef DEBUGALL
    cout << "Command executed " << command.str() << endl;
#endif
    char *compare = new char[256];
    compare[0] = '\0';
    char **compptr = &compare;
    size_t size = 256;
    getline(compptr,&size,pipe);
//    fscanf(pipe,"%s",compare);
    pclose(pipe);
    if(strlen(compare))
    {
      cout << __PRETTY_FUNCTION__ << " The writing process produced an error for class " << task->ClassId() << compare << endl;
    }
    delete []compare;    
  }
#endif
  // I dont need to lock if I am the service thread
  // (in that case I already have the lock)
  if(!pthread_equal(fExecuteThread,pthread_self()))
  { 
    pthread_mutex_lock(&fSubmittedMutex);
  }
  fSubmittedList.push_back (task);
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
    pthread_cond_signal(&fExecuteCondition);
    pthread_mutex_unlock(&fSubmittedMutex);
  }
  return id;
}
int OOPTaskManager::NumberOfTasks ()
{
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_DEBUG (logger,sout.str());
#endif    
  }
	int numtasks = fExecutable.size () + fFinished.size () +
		fSubmittedList.size () + fTaskList.size () + fDaemon.size();
	return numtasks;
}
bool OOPTaskManager::HasWorkTodo ()
{
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_DEBUG (logger,sout.str());
#endif
    return false;
  }
  int numtasks = fExecutable.size () + fFinished.size () + fSubmittedList.size () + fDaemon.size();
  return numtasks != 0;
}


int OOPTaskManager::ChangePriority (OOPObjectId & taskid, int newpriority)
{
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_DEBUG (logger,sout.str());
#endif    
  }
	OOPTask *t = FindTask (taskid);
	if (t) {
		t->ChangePriority (newpriority);
		Reschedule ();
	}
	return 0;
}
int OOPTaskManager::CancelTask (OOPObjectId taskid)
{
  // I assume that I already have the lock!!
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_ERROR (logger,sout.str());
#endif    
  }
  
  list < OOPTaskControl * >::iterator i;	// , iprev, atual;
  for (i = fTaskList.begin (); i != fTaskList.end (); i++)
  {
    OOPTaskControl *tc = *i;
    if (tc->Task ()->Id () == taskid)
    {
#ifdef LOGPZ      
      stringstream sout;
      sout << "Task erased ";
      sout << "Task ID " << tc->Task()->Id() << " classid " << tc->Task()->ClassId();
      LOGPZ_DEBUG (logger,sout.str());
#endif      
      tc->Depend ().ReleaseAccessRequests (tc->Task ()->Id ());
      delete tc;
      fTaskList.erase (i);
      return 1;
    }
  }
  return 0;
}
void OOPTaskManager::ExecuteDaemons() {
  
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread fExecuteThread " << fExecuteThread
    << " thread_self " << pthread_self();
    LOGPZ_DEBUG (logger,sout.str());
#endif    
  }
	list < OOPDaemonTask * >::iterator i;
	while(fDaemon.size()) {
		i = fDaemon.begin();
		if((*i)->GetProcID() != DM->GetProcID()) {
			CM->SendTask((*i));
		} else {
			(*i)->Execute();
			delete (*i);
		}
		fDaemon.erase(i);
	}
}
void OOPTaskManager::Execute ()
{
#ifndef MT
	//Qual �o service thread ?
	// O service thread e a linha de execucao do programa principal
	DM->SubmitAllObjects();
	CM->ReceiveMessages ();
	TransferSubmittedTasks ();
	list < OOPTaskControl * >::iterator i;
	fKeepGoing=true;
	ExecuteDaemons();
	while (fKeepGoing) {
		//pthread_mutex_lock(&fExecuteMutex);
		DM->SubmitAllObjects();
		CM->ReceiveMessages();
		ExecuteDaemons();
		while (fExecutable.size ()) {
			//pthread_mutex_unlock(&fExecuteMutex);
			i = fExecutable.begin ();
			OOPTaskControl *tc = (*i);
			tc->Task ()->Execute ();
			OOPObjectId id;
			id = tc->Task ()->Id ();
			tc->Depend ().SetExecuting (tc->Task ()->Id (),
						      false);
			tc->Depend ().ReleaseAccessRequests (tc->Task ()->
							       Id ());
			DM->SubmitAllObjects();

			fFinished.push_back (tc);
			fExecutable.erase (i);
		}
		DM->SubmitAllObjects();
		TransferFinishedTasks ();
		CM->ReceiveMessages ();
		TransferSubmittedTasks ();
		CM->SendMessages ();
		ExecuteDaemons();
		//wait
		pthread_mutex_lock(&fSubmittedMutex);
		if(!HasWorkTodo () && fKeepGoing){
//			cout << "Going into Blocking receive on TM->Execute()\n";
//			cout << "PID" << getpid() << endl;
			cout.flush();
                        pthread_mutex_unlock(&fSubmittedMutex);
#ifdef OOP_MPI
			OOPMPICommManager *MPICM = dynamic_cast<OOPMPICommManager *> (CM);
			if(MPICM) MPICM->ReceiveBlocking();
			#endif
//			pthread_cond_wait(&fExecuteCondition, &fExecuteMutex);
#ifdef LOGPZ  
                        stringstream sout;
			sout << "Leaving blocking receive PID " << getpid();
                        LOGPZ_DEBUG (logger,sout.str());
#endif      
			DM->SubmitAllObjects();
                        pthread_mutex_lock(&fSubmittedMutex);
                }
                pthread_mutex_unlock(&fSubmittedMutex);
	}
	CM->SendMessages ();
#else
//	pthread_t execute_thread;
  {
#ifdef LOGPZ    
    stringstream sout;
	  sout << "Creating service thread";
    LOGPZ_DEBUG (logger,sout.str());
#endif    
  }
	if(pthread_create(&fExecuteThread, NULL, ExecuteMT, this)){
#ifdef LOGPZ    
    stringstream sout;
    sout << "Fail to create service thread\n";
    sout << "Going out";
    LOGPZ_ERROR (logger,sout.str());
#endif    
	} 
 {
#ifdef LOGPZ   
   stringstream sout;
   sout << "Created succesfuly";
   LOGPZ_DEBUG (logger,sout.str());
#endif   
 }
#endif
}
void OOPTaskManager::Wait(){
	pthread_join(fExecuteThread,NULL);
}
void OOPTaskManager::SetKeepGoing(bool go){
	fKeepGoing = go;
}
OOPObjectId OOPTaskManager::GenerateId ()
{      // Generate a unique id number
	fLastCreated++;
//	if (fLastCreated >= fMaxId)
// {
//   cout << __PRETTY_FUNCTION__ << " available Ids exhausted bye!\n";
//    exit (-1); // the program ceases to function
//  }
	OOPObjectId tmp (fProc, fLastCreated);
	return tmp;
}
OOPTask *OOPTaskManager::FindTask (OOPObjectId taskid)
{      // find the task with the given id
	list < OOPTaskControl * >::iterator i;
	for (i = fTaskList.begin (); i != fTaskList.end (); i++) {
		OOPTask *t = (*i)->Task ();
		if (t->Id () == taskid)
			return t;
	}
	return 0;
}
/**
 * reorder the tasks according to their priority
 * No local implementation found
 */
void OOPTaskManager::Reschedule ()
{
}
void OOPTaskManager::Print (std::ostream & out)
{
	out << "Task Manager data structure \t" << endl;
	out << "TM Processor " << fProc << endl;
	out << "Id of Last Created Task \t" << fLastCreated << endl;
//	out << "Maximum number of tasks available \t" << fMaxId << endl;
	out << "Queued Daemon tasks ---------\t" << endl;
	out << "Queued Time Consuming tasks ---------" << endl;
	out << "Number of Time Consuming tasks \t" << fTaskList.
		size () << endl;
	list < OOPTaskControl * >::iterator i;
	for (i = fTaskList.begin (); i != fTaskList.end (); i++) {
		(*i)->Task ()->Print (out);
	}
}

void OOPTaskManager::TransferSubmittedTasks ()
{
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_DEBUG (logger,sout.str());
#endif    
  }
  list < OOPTask * >::iterator sub;
  DM->SubmitAllObjects();

  int listsize = fSubmittedList.size();
  sub = fSubmittedList.begin ();
  OOPTask * aux = 0;
  if(listsize)
  {
    aux = (*sub);
    fSubmittedList.erase (sub);
  }
	
  while (aux)
  {
    //(fSubmittedList.begin () != fSubmittedList.end ()) {
    //sub = fSubmittedList.begin ();
    //OOPTask * aux = (*sub);
    OOPDaemonTask *dmt = dynamic_cast < OOPDaemonTask * >(aux);
    if (aux->GetProcID () != fProc)
    {
#ifdef LOGPZ      
      stringstream sout;
      sout << __PRETTY_FUNCTION__ << "Transferring task " << aux->Id() << " from " << fProc <<
           " to proc " <<  aux->GetProcID();
      LOGPZ_DEBUG(tasklogger,sout.str())
#endif      
      CM->SendTask (aux);
    }
    else if(dmt)
    {
      SubmitDaemon(dmt);
    }
    else
    {
      OOPTaskControl *tc = new OOPTaskControl (aux);
      fTaskList.push_back (tc);
      if (tc->Depend ().SubmitDependencyList (tc->Task ()->Id ()))
      {
        // their is no incompatibility between
        // versions
      }
      else
      {
        // there is an incompatibility of versions
        CancelTask (tc->Task ()->Id ());
      }
    }
    DM->SubmitAllObjects();
    listsize = fSubmittedList.size();
    sub = fSubmittedList.begin ();
    aux = 0;
    if(listsize)
    {
      aux = (*sub);
      fSubmittedList.erase (sub);
    }
  }
}
void OOPTaskManager::TransferFinishedTasks ()
{
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_DEBUG (logger,sout.str());
#endif    
  }
  list < OOPTaskControl * >::iterator sub;
  int listsize = fFinished.size();
  if(!listsize) return;
  DM->SubmitAllObjects();
  sub = fFinished.begin();
  OOPTaskControl * auxtc=0;
  if(listsize)
  {
    auxtc = (*sub);
    fFinished.erase(sub);
  }
  while (auxtc)
  {
#ifdef LOGPZ
    {
      stringstream sout;
      sout << __PRETTY_FUNCTION__ << " task " << auxtc->Task()->Id() << " classid " << auxtc->Task()->ClassId() << " finished";
      LOGPZ_DEBUG(tasklogger,sout.str());
    }
#endif
    if (auxtc->Task ()->IsRecurrent () && auxtc->Task ()->GetProcID () != fProc)
    {
#ifdef LOGPZ
      {
        stringstream sout;
        sout << __PRETTY_FUNCTION__ << " task " << auxtc->Task()->Id() << " classid " <<
            auxtc->Task()->ClassId() << " transferred from " << fProc << " to " << auxtc->Task ()->GetProcID();
        LOGPZ_DEBUG(tasklogger,sout.str());
      }
#endif
      CM->SendTask (auxtc->Task ());
      auxtc->ZeroTask ();
      delete auxtc;
    }
    else if(auxtc->Task ()->IsRecurrent ())
    {
#ifdef LOGPZ
      {
        stringstream sout;
        sout << __PRETTY_FUNCTION__ << " task " << auxtc->Task()->Id() << " classid " <<
            auxtc->Task()->ClassId() << " resubmitted";
        LOGPZ_DEBUG(tasklogger,sout.str());
      }
#endif
      auxtc->Depend () = auxtc->Task ()->GetDependencyList ();
      auxtc->Depend ().ClearPointers ();
      fTaskList.push_back (auxtc);
      if (auxtc->Depend ().SubmitDependencyList (auxtc->Task ()->Id ()))
      {
        // their is no incompatibility between
        // versions
      }
      else
      {
        // there is an incompatibility of versions
        CancelTask (auxtc->Task ()->Id ());
      }
    }
    else
    {
      delete auxtc;
    }
    listsize = fFinished.size();
    auxtc=0;
    sub = fFinished.begin ();
    if(listsize)
    {
      auxtc = (*sub);
      fFinished.erase (sub);
    }
  }
}

void OOPTaskManager::TransfertoExecutable (const OOPObjectId & taskid)
{
  if(!pthread_equal(fExecuteThread,pthread_self()))
  {
#ifdef LOGPZ    
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_DEBUG (logger,sout.str());
#endif    
  }
  list < OOPTaskControl * >::iterator i;
  for (i = fTaskList.begin (); i != fTaskList.end (); i++) {
    OOPTaskControl *tc = (*i);
    if (tc->Task ()->Id () == taskid) {
      tc->Task ()->SetDependencyList (tc->Depend ());
      tc->Depend ().SetExecuting (taskid, true);
      OOPDaemonTask *dmt = dynamic_cast < OOPDaemonTask * >(tc->Task ());
      if (dmt) {
#ifdef LOGPZ        
        LOGPZ_ERROR (logger,"TM::TransfertoExecutable inconsistent datastructure\nThere is daemontask in the fTaskList\n");
#endif        
          SubmitDaemon(dmt);
          tc->ZeroTask();
          delete tc;
        }
        else {
          fExecutable.push_back (tc);
        }
        fTaskList.erase (i);
        break;
      }
    }
}

void OOPTaskManager::PrintTaskQueues(char * msg, std::ostream & out){
	out << msg << endl;
	out << "Printing TaskManager Queues on TM:" << fProc << endl;
	out << "Print fSubmittedList\n";
	out << "Number of tasks :" << fSubmittedList.size() << endl;
	list < OOPTask * >::iterator i;
	for(i=fSubmittedList.begin();i!=fSubmittedList.end();i++)
		out << (*i)->Id() << endl;
	out << "Print fTaskList\n";
	out << "Number of tasks :" << fTaskList.size() << endl;
	list < OOPTaskControl * >::iterator j;
	for(j=fTaskList.begin();j!=fTaskList.end();j++)
		out << (*j)->Task()->Id() << endl;
	out << "Print fExecutable\n";
	out << "Number of tasks :" << fExecutable.size() << endl;
	for(j=fExecutable.begin();j!=fExecutable.end();j++)
		out << (*j)->Task()->Id() << endl;
	out << "Print fFinished\n";
	out << "Number of tasks :" << fFinished.size() << endl;
	for(j=fFinished.begin();j!=fFinished.end();j++)
		out << (*j)->Task()->Id() << endl;
	
}
OOPTMTask::OOPTMTask(): OOPDaemonTask() {
  LOGPZ_DEBUG (logger,"Should Never be Called\n");
}
OOPTMTask::~OOPTMTask(){}
OOPMReturnType OOPTMTask::Execute (){
  LOGPZ_DEBUG (logger,"Should Never be Called\n");
  return EContinue;
}


OOPTerminationTask::~OOPTerminationTask (){}
OOPTerminationTask::OOPTerminationTask (int ProcId) : OOPTask(ProcId){}
OOPTerminationTask::OOPTerminationTask (const OOPTerminationTask & term): OOPTask(term)
{
}

OOPMReturnType OOPTerminationTask::Execute (){
  TM->SetKeepGoing(false);  
  LOGPZ_DEBUG (logger,"----------------------------------------------TM Finished\n");
	return ESuccess;
}

void OOPTerminationTask::Write(TPZStream & buf, int withclassid){
	OOPTask::Write(buf, withclassid);

}
void OOPTerminationTask::Read(TPZStream & buf, void * context){
	OOPTask::Read(buf, context);

}

long int OOPTerminationTask::ExecTime(){
	return -1;
}
TPZSaveable *OOPTerminationTask::Restore (TPZStream & buf, void * context){
	OOPTerminationTask*v = new OOPTerminationTask(0);
	v->Read (buf);
	return v;
}

void OOPTaskManager::Lock(TMLock &lock)
{
  if(pthread_equal(pthread_self(),fLockThread))
  {
    std::cout << "A single thread locking twice\n";
    return;
  }
  pthread_mutex_lock(&fSubmittedMutex);
  fLockThread = pthread_self();
  if(fLock)
  {
    std::cout << __PRETTY_FUNCTION__ << " Lock passed on a already lock mutex\n";
    return;
  }
  fLock = &lock;
}

void OOPTaskManager::Unlock(TMLock &lock)
{
  if(fLock != &lock)
  {
    std::cout << __PRETTY_FUNCTION__ << " Unlock called for the wrong lock object\n";
    return;
  }
  pthread_mutex_unlock(&fSubmittedMutex);
  fLock = 0;
  fLockThread = 0;
}

void OOPTaskManager::Signal(TMLock &lock)
{
  if(fLock != &lock)
  {
    std::cout << __PRETTY_FUNCTION__ << " Signal called for the wrong lock object\n";
    return;
  }
  pthread_cond_signal(&fExecuteCondition);
}

TMLock::TMLock()
{
  TM->Lock(*this);
}

TMLock::~TMLock()
{
  TM->Unlock(*this);
}

void TMLock::Signal()
{
  TM->Signal(*this);
}

extern OOPTaskManager *TM;
