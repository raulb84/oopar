#ifdef OOP_MPI
#include "oopmpicomm.h"
#endif


#include <errno.h>

#include "ooptaskmanager.h"
#include "oopcommmanager.h"
#include "oopdatamanager.h"
#include "ooptaskcontrol.h"

#include "oopwaittask.h"

#include "ooptmlock.h"
#include "oopdmlock.h"
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sstream>
#include <iostream>

#include <sys/time.h>

#ifdef OOP_MPE
class OOPSoloEvent;
#endif
class OOPStorageBuffer;
class OOPTask;
class OOPDataVersion;
class OOPSaveable;
class OOPObjectId;
class OOPTerminationTask;

#include <sstream>
#include <pzlog.h>
#ifdef LOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
using namespace log4cxx;
using namespace log4cxx::helpers;

static LoggerPtr logger (Logger::getLogger ("OOPar.OOPTaskManager"));
static LoggerPtr tasklogger (Logger::getLogger ("OOPar.OOPTaskManager.OOPTask"));
static LoggerPtr ServiceLogger (Logger::getLogger ("OOPar.OOPTaskManager.ServiceThread"));

#endif

static ofstream
tlog ("TM_time_log.txt");


void OOPTaskManager::SnapShotMe()
{
  stringstream filename;
  filename << "TMSnapShot" << fProc << ".txt";
  ofstream out(filename.str().c_str(), ios::app);
  std::stringstream sout;
  sout << "TaskManager SnapShot\n";
  sout << "Processor" << fProc << endl;
  sout << "Executable size " << fExecutable.size() << endl;
  sout << "TaskList size " << fTaskList.size() << endl;
  
  {
    sout << "---Executable---" << endl;
    std::list<OOPTaskControl * >::iterator lit;
    for(lit=fExecutable.begin();lit!=fExecutable.end();lit++)
    {
      (*lit)->Print(sout);
    }
  }
  {
    sout << "---Executing---" << endl;
    std::list<OOPTaskControl * >::iterator lit;
    for(lit=fExecuting.begin();lit!=fExecuting.end();lit++)
    {
      (*lit)->Print(sout);
    }
  }
  {
    sout << "---TaskList---" << endl;
    std::list<OOPTaskControl * >::iterator lit;
    for(lit=fTaskList.begin();lit!=fTaskList.end();lit++)
    {
      (*lit)->Print(sout);
    }
  }
  
  sout << "---------Finished STL Data Components------------" << endl;
  out << sout.str().c_str();
}


void OOPTaskManager::GrantAccess(OOPAccessTag & tag)
{
  std::pair< TMMessageType, OOPAccessTag> item(ETMAccessGranted, tag);
#ifdef LOGPZ
  stringstream sout;
  sout << "Task " << tag.TaskId() << " Received Access from Request " << tag.AccessModeString();
  LOGPZ_DEBUG (logger, sout.str ());
#endif
  OOPTMLock lock;
  fMessages.push_back(item);
}
void
OOPTaskManager::SetNumberOfThreads (const int n)
{
  OOPTaskManager::fNumberOfThreads = n;
}

int
OOPTaskManager::NumberOfThreads ()
{
  return OOPTaskManager::fNumberOfThreads;
}

void OOPTaskManager::TransferExecutingTasks ()
{
  if (!pthread_equal (fExecuteThread, pthread_self ())) {
#ifdef LOGPZ
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_ERROR (logger, sout.str ());
#endif
    return;
  }
  list < OOPTaskControl * >::iterator sub;
  sub = fExecuting.begin ();
  OOPTaskControl *auxtc = 0;
  while (sub != fExecuting.end ()) {
    bool isfinished = false;
    auxtc = (*sub);
    {
      OOPTMLock lock;
      if (auxtc) {
        isfinished = auxtc->TaskFinished ();
      } else {
  #ifdef LOGPZ
        stringstream sout;
        sout << __FUNCTION__ << " I dont understand \n";
        LOGPZ_ERROR (logger, sout.str ());
  #endif
      }
    }
    
    if (isfinished) {
#ifdef LOGPZ
      {
      stringstream sout;
      sout << "Task " << auxtc->Id() << " Finshed\nCalling TaskControl->Join()\n";
      LOGPZ_DEBUG (logger, sout.str ());
      }
#endif
    
      auxtc->Join ();
      DM->WakeUpCall();
      WakeUpCall();
#ifdef LOGPZ
      {
      stringstream sout;
      sout << "TaskControl::ThreadExec Joined ServiceThread\n";
      sout << "Task finished " << auxtc->
	Id () << " classid " << auxtc->ClassId ();
      LOGPZ_DEBUG (tasklogger, sout.str ());
      }
#endif
      delete auxtc;
      list < OOPTaskControl * >::iterator keep;
      keep = sub;
      fExecuting.erase (keep);
      sub = fExecuting.begin ();
    } else {
      sub++;
    }
  }
}


void *
OOPTaskManager::ExecuteMTBlocking (void *data)
{
  OOPTaskManager *lTM = static_cast < OOPTaskManager * >(data);
  OOPMPICommManager * MPICM = NULL;
  MPICM = static_cast<OOPMPICommManager *>(CM);
  if(!MPICM)
  {
#ifdef LOGPZ
    stringstream sout;
    sout << "MPICommManager not valid ! Bailing out\nRETURNING NULL FROM SERVICE THREAD!\nFAREWELL" << endl;
    LOGPZ_ERROR(ServiceLogger, sout.str());
#endif
    return NULL;
  }
  if(MPICM->ReceiveMessagesBlocking())
  {
#ifdef LOGPZ
    stringstream sout;
    sout << "MPICommManager Could not initialize the receiving thread !\nRETURNING NULL FROM SERVICE THREAD!\nFAREWELL" << endl;
    LOGPZ_ERROR(ServiceLogger, sout.str());
#endif
    return NULL;
  }
  {
#ifdef LOGPZ
    {
      stringstream sout;
      sout << "CM->ReceiveThread Triggered Sucessfully";
      LOGPZ_DEBUG(ServiceLogger, sout.str());
    }
    {
      stringstream sout;
      sout << "Setting KeepGoing Flag to true and entering TM Inifinit loop";
      LOGPZ_DEBUG(ServiceLogger, sout.str());
    }
#endif
  }
  DM->StartService();
  lTM->SetKeepGoing( true);
  
  while (lTM->KeepRunning())
  {
/*    {
      stringstream sout;
      sout << "Calling DM->HandleMessages()";
      #ifdef LOGPZ
      LOGPZ_DEBUG(ServiceLogger, sout.str());
      #endif
    }
    DM->HandleMessages();*/
    {
      stringstream sout;
      sout << "Called DM->HandleMessages() | Calling lTM->HandleMessages()";
      #ifdef LOGPZ
      LOGPZ_DEBUG(ServiceLogger, sout.str());
      #endif
    }
    lTM->HandleMessages();
    {
      stringstream sout;
      sout << "Called lTM->HandleMessages() | Calling DM->FlushData()";
      #ifdef LOGPZ
      LOGPZ_DEBUG(ServiceLogger, sout.str());
      #endif
    }
/*    DM->FlushData();
    {
      stringstream sout;
      sout << "Called DM->FlushData() | Calling lTM->TriggerTasks()";
      #ifdef LOGPZ
      LOGPZ_DEBUG(ServiceLogger, sout.str());
      #endif
    }*/
    lTM->TriggerTasks();
    {
      stringstream sout;
      sout << "Called lTM->TriggerTasks() | Going to Sleep";
      #ifdef LOGPZ
      LOGPZ_DEBUG(ServiceLogger, sout.str());
      #endif
    }
    lTM->WaitWakeUpCall();
    {
      stringstream sout;
      sout << "Woke Up | One more round ------------------------------------------------------------------" << endl;
      #ifdef LOGPZ
      LOGPZ_DEBUG(ServiceLogger, sout.str());
      #endif
    }
  }
  {
#ifdef LOGPZ
  stringstream sout;
  sout << "Leaving TM Inifinit loop\nHopefully TM was shutdown by a TerminationTask." << endl;
  LOGPZ_DEBUG(logger, sout.str());
#endif
  }
  return NULL;  
}
void OOPTaskManager::TriggerTasks()
{
  std::list< OOPTaskControl * >::iterator i;
  //while (fExecutable.size()  && (int) fExecuting.size()) 
#ifdef LOGPZ
  {
    stringstream sout;
    sout << "TriggerTask : fExecutable.size() = " << fExecutable.size(); 
    LOGPZ_DEBUG (tasklogger, sout.str ());
  }
#endif
  while ((int)fExecutable.size())//  || (int)fExecuting.size())
  {
    i = fExecutable.begin ();
    OOPTaskControl *tc = (*i);
    fExecutable.erase (i);
    fExecuting.push_back (tc);
#ifdef LOGPZ
    {
      stringstream sout;
      sout << "Entering taskcontrol execute for task " << tc->
        Id () << " classid " << tc->ClassId ();
      LOGPZ_DEBUG (tasklogger, sout.str ());
    }
#endif
    tc->Execute ();
    TransferExecutingTasks ();
  }
  TransferExecutingTasks ();
}
void OOPTaskManager::WaitWakeUpCall()
{
#ifdef LOGPZ
  {
    stringstream sout;
    sout << "Going to sleep in " << __PRETTY_FUNCTION__ << endl;
    LOGPZ_DEBUG (tasklogger, sout.str ());
  }
#endif
//#warning "Non stop service thread defined"
//#define NONSTOPSERVICETHREAD
#ifndef NONSTOPSERVICETHREAD
  sem_wait(&fServiceSemaphore);
#else
  timeval now;
  gettimeofday (&now, 0);
  now.tv_usec += 1000*10;
  now.tv_sec += now.tv_usec / 1000000;
  now.tv_usec %= 1000000;
  timespec next;
  next.tv_sec = now.tv_sec+1;
  next.tv_nsec = now.tv_usec;// * 1000;
	
  int retval = 0;
  retval = sem_timedwait(&fServiceSemaphore, &next);

  if(retval == ETIMEDOUT){
#ifdef LOGPZ  
    LOGPZ_DEBUG(ServiceLogger,"Sem_TimedWait TimedOut");
#endif
  }else{
#ifdef LOGPZ  
    LOGPZ_DEBUG(ServiceLogger,"Sem_TimedWait Signaled ");
#endif
  }
  
#endif
#ifdef LOGPZ
  {
    stringstream sout;
    sout << "Awaken from sem_wait in " << __PRETTY_FUNCTION__ << endl;
    LOGPZ_DEBUG (tasklogger, sout.str ());
  }
#endif
}

bool OOPTaskManager::KeepRunning()
{
  bool result = false;
  if(fKeepGoing)
  {
    result = true;
  }
  if(fTaskList.size())
  {
    result = true;
  }
  if(fExecutable.size())
  {
    result = true;
  }
  if(fExecuting.size())
  {
    result = true;
  }
  if(fMessages.size())
  {
    result = true;
  }
  return result;
}



OOPTaskManager::OOPTaskManager (int proc):fNumberOfThreads (10)
{
  fProc = proc;
  fLastCreated = 0;		//NUMOBJECTS * fProc;
  fExecuteThread = 0;
  stringstream filename;
  filename << "TaskMainLog" << fProc << ".dat";
  fMainLog = 0;
  sem_init(&fServiceSemaphore, 0, 0);
}

OOPTaskManager::~OOPTaskManager ()
{
#ifdef LOGPZ
	stringstream sout;
	sout << "Executable.size " << fExecutable.size() << 
	" tasklist.size " << fTaskList.size();
	LOGPZ_DEBUG (tasklogger, sout.str ());
#endif
  list < OOPTaskControl * >::iterator itc;
  for (itc = fExecutable.begin (); itc != fExecutable.end (); itc++)
    delete *itc;
  for (itc = fTaskList.begin (); itc != fTaskList.end (); itc++)
    delete *itc;
}
void
OOPTaskManager::SubmitDaemon (OOPDaemonTask * task)
{
  if (task->GetProcID () != this->fProc) {
#ifdef LOGPZ
    stringstream sout;
    sout << "Sending a daemon task to proc " << task->
      GetProcID () << " classid " << task->ClassId ();
#endif
    CM->SendTask (task);
#ifdef LOGPZ
    LOGPZ_DEBUG (logger, sout.str ());
#endif

  } else {
#ifdef LOGPZ
    stringstream sout;
    sout << "Executting a daemon task " << " classid "
      << task->ClassId ();
    LOGPZ_DEBUG (logger, sout.str ());
#endif
    {
      OOPTMLock lock;
      ExecuteDaemon(task);
    }
    DM->WakeUpCall();
    WakeUpCall();
  }
}

OOPObjectId OOPTaskManager::Submit (OOPTask * task)
{

  OOPDaemonTask *dmt = dynamic_cast < OOPDaemonTask * >(task);
  OOPObjectId id;
  if (dmt) {
#ifdef LOGPZ
    stringstream sout;
    sout << "Task Submitted is a daemon classid = " << task->ClassId();
    LOGPZ_DEBUG (logger, sout.str ());
#endif
    SubmitDaemon(dmt);
    return id;
  } else {
    id = task->Id ();
    if (id.IsZeroOOP ())
    {
      id = GenerateId ();
    }
    task->SetTaskId (id);
  }
  {
#ifdef LOGPZ
    stringstream sout;
    sout << "Task with id " << task->
      Id () << " submitted for processor " << task->
      GetProcID () << " classid " << task->ClassId ();
    LOGPZ_DEBUG (tasklogger, sout.str ());
#endif
  }
#ifdef DEBUGALL
  OOPWaitTask *wait = dynamic_cast < OOPWaitTask * >(task);
  if (!wait && !dmt && !CM->GetProcID ()) {
    std::ostringstream FileName, FileName2, command, subdir1, subdir2,
      subdir3;
    subdir1 << "taskman" << CM->GetProcID ();
    subdir2 << "taskman" << CM->GetProcID () << "/orig";
    subdir3 << "taskman" << CM->GetProcID () << "/copy";
    mkdir (subdir1.str ().c_str (),
	   S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH);
    mkdir (subdir2.str ().c_str (),
	   S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH);
    mkdir (subdir3.str ().c_str (),
	   S_IRWXU | S_IXGRP | S_IRGRP | S_IXOTH | S_IROTH);
    FileName << subdir2.str () << "/" << task->ClassId () << ".sav";
    FileName2 << subdir3.str () << "/" << task->ClassId () << ".sav";
    {
      TPZFileStream PZFS;
      PZFS.OpenWrite (FileName.str ());
      task->Write (PZFS, 1);
    }
    {
      TPZFileStream PZFS;
      PZFS.OpenRead (FileName.str ());
      OOPTask *test = (OOPTask *) TPZSaveable::Restore (PZFS, 0);
      TPZFileStream PZFS2;
      PZFS2.OpenWrite (FileName2.str ());
      test->Write (PZFS2, 1);
      delete test;
    }
    command << "diff --brief " << FileName.str () << " " << FileName2.
      str () << endl;
    FILE *pipe = popen (command.str ().c_str (), "r");
#ifdef DEBUGALL
    cout << "Command executed " << command.str () << endl;
#endif
    char *compare = new char[256];
    compare[0] = '\0';
    char **compptr = &compare;
    size_t size = 256;
    getline (compptr, &size, pipe);
    pclose (pipe);
    if (strlen (compare)) {
      cout << __PRETTY_FUNCTION__ <<
	" The writing process produced an error for class " << task->
	ClassId () << compare << endl;
    }
    delete [] compare;
  }
#endif
  if(task->GetProcID() %CM->NumProcessors() != CM->GetProcID())
  {
#ifdef LOGPZ
    stringstream sout;
    sout << "Task Being sent to processor " << task->GetProcID();
    LOGPZ_DEBUG (logger, sout.str ());
#endif
    CM->SendTask(task);
  }else
  {
    OOPTMLock lock;
    InsertTask(task);
  }
  DM->WakeUpCall();
  WakeUpCall();
  return id;
}
void OOPTaskManager::InsertTask(OOPTask * task)
{
  OOPDaemonTask *dmt = dynamic_cast < OOPDaemonTask * >(task);
  if (dmt)
  { //Checks if dmt is valid. aux was a DaemonTask
#ifdef LOGPZ
    stringstream sout;
    sout << "Submitting Daemon task on TransferSubmitted Tasks";
    LOGPZ_ERROR (tasklogger, sout.str ())
#endif        
    SubmitDaemon (dmt);
  } else {//Ordinary task to be executed in this processor
    task->SubmitDependencyList();
  #ifdef LOGPZ
    stringstream sout;
    sout << "Creating the task control ojbect for task " << task->Id () ;
    LOGPZ_DEBUG (tasklogger, sout.str ())
  #endif

  
    OOPTaskControl *tc = new OOPTaskControl(task);
    if(task->CanExecute())
    {
      fExecutable.push_back(tc);
#ifdef LOGPZ
      stringstream sout;
      sout << "Task is Executable " << task->Id () ;
      LOGPZ_DEBUG (tasklogger, sout.str ())
#endif
      WakeUpCall();
      DM->WakeUpCall();
    }else
    {
      fTaskList.push_back (tc);
    }
  }
}
int OOPTaskManager::NumberOfTasks ()
{
  if (!pthread_equal (fExecuteThread, pthread_self ())) {
#ifdef LOGPZ
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_DEBUG (logger, sout.str ());
#endif
  }
  int numtasks = fExecutable.size () + fTaskList.size ();
  return numtasks;
}

void OOPTaskManager::CancelTask (OOPAccessTag & tag)
{
  std::pair<TMMessageType, OOPAccessTag> item(ETMCancelTask, tag);
  OOPTMLock lock;
  fMessages.push_back(item);
}
void OOPTaskManager::ExecuteDaemon(OOPTask * dmt)
{
#ifdef LOGPZ
  OOPDMRequestTask * req = dynamic_cast<OOPDMRequestTask *>(dmt);
  if(req)
  {
    stringstream sout;
    sout << "Daemon Task is a RequestTask with tag ";
    req->fDepend.ShortPrint(sout);
    LOGPZ_DEBUG(logger, sout.str());
  }
  OOPDMOwnerTask * own = dynamic_cast<OOPDMOwnerTask *>(dmt);
  if(own)
  {
    stringstream sout;
    sout << "Daemon Task is a OwnerTask with tag ";
    own->fTag.ShortPrint(sout);
    LOGPZ_DEBUG(logger, sout.str());
  }
#endif    
  if (dmt->GetProcID () != DM->GetProcID ())
  {
#ifdef LOGPZ
    {
      stringstream sout;
      sout << "Daemon task is for a different processor ! Calling CM->SendTask()"; 
      LOGPZ_DEBUG (logger, sout.str ());
    }
#endif
    CM->SendTask (dmt);
  } else {
    dmt->Execute ();
    delete dmt;
  }
}
void OOPTaskManager::Execute ()
{
//      pthread_t execute_thread;
  {
#ifdef LOGPZ
    stringstream sout;
    sout << "Creating service thread";
    LOGPZ_DEBUG (logger, sout.str ());
#endif
  }
#ifdef BLOCKING
   if (pthread_create (&fExecuteThread, NULL, ExecuteMTBlocking, this)) {
#else
   if (pthread_create (&fExecuteThread, NULL, ExecuteMT, this)) {
#endif
#ifdef LOGPZ
    stringstream sout;
    sout << "Fail to create service thread\n";
    sout << "Going out";
    LOGPZ_ERROR (logger, sout.str ());
#endif
  }
  {
#ifdef LOGPZ
    stringstream sout;
    sout << "Created succesfuly";
    LOGPZ_DEBUG (logger, sout.str ());
#endif
  }
}

void
OOPTaskManager::Wait ()
{

  pthread_join (fExecuteThread, NULL);
#ifdef LOGPZ
  {
    stringstream sout;
    sout << "ServiceThread on Processor " << CM->GetProcID() << " joined main application execution thread\n";
    sout << "TM's ServiceThread no longer exists !";
    
    LOGPZ_DEBUG (logger, sout.str ());
    cout << sout.str() << endl;
  }
#endif
#ifdef BLOCKING
  MPI_Barrier(MPI_COMM_WORLD);
  ((OOPMPICommManager *)CM)->UnlockReceiveBlocking(); 
#endif
#ifdef LOGPZ
  {
    stringstream sout;
    sout << "Synchronizing MPI environment with MPI_Barrier on Processor " << CM->GetProcID();
    LOGPZ_DEBUG (logger, sout.str ());
    cout << sout.str() << endl;
  }
#endif
  MPI_Barrier(MPI_COMM_WORLD);
#ifdef LOGPZ
  {
    stringstream sout;
    sout << "Leaving MPI_Barrier on processor " << CM->GetProcID();
    LOGPZ_DEBUG (logger, sout.str ());
    cout << sout.str() << endl;
  }
#endif
}

void
OOPTaskManager::SetKeepGoing (bool go)
{
  fKeepGoing = go;
}

OOPObjectId
OOPTaskManager::GenerateId ()
{
  fLastCreated++;
  OOPObjectId tmp (fProc, fLastCreated);
  return tmp;
}

OOPTask *
OOPTaskManager::FindTask (OOPObjectId taskid)
{				// find the task with the given id
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
void
OOPTaskManager::Print (std::ostream & out)
{
  out << "Task Manager data structure \t" << endl;
  out << "TM Processor " << fProc << endl;
  out << "Id of Last Created Task \t" << fLastCreated << endl;

  out << "Queued Daemon tasks ---------\t" << endl;
  out << "Queued Time Consuming tasks ---------" << endl;
  out << "Number of Time Consuming tasks \t" << fTaskList.size () << endl;
  list < OOPTaskControl * >::iterator i;
  for (i = fTaskList.begin (); i != fTaskList.end (); i++) {
    (*i)->Task ()->Print (out);
  }
}

void OOPTaskManager::TransfertoExecutable (const OOPObjectId & taskid)
{
  if (!pthread_equal (fExecuteThread, pthread_self ())) {
#ifdef LOGPZ
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_DEBUG (logger, sout.str ());
#endif
  }
  list < OOPTaskControl * >::iterator i;
  {
    OOPTMLock lock;
    for (i = fTaskList.begin (); i != fTaskList.end (); i++) {
      OOPTaskControl *tc = (*i);
      if (tc->Task ()->Id () == taskid) {
        OOPDaemonTask *dmt = dynamic_cast < OOPDaemonTask * >(tc->Task ());
        if (dmt) {
  #ifdef LOGPZ
          LOGPZ_ERROR (logger,
                       "TM::TransfertoExecutable inconsistent datastructure\nThere is daemontask in the fTaskList\n");
  #endif
          SubmitDaemon (dmt);
          tc->ZeroTask ();
          delete tc;
        } else {
          fExecutable.push_back (tc);
        }
        fTaskList.erase (i);
        //WakeUpCall();
        break;
      }
    }
    //WakeUpCall();
  }
}

void
OOPTaskManager::PrintTaskQueues (char *msg, std::ostream & out)
{
  out << msg << endl;
  out << "Printing TaskManager Queues on TM:" << fProc << endl;
  list < OOPTask * >::iterator i;
  out << "Print fTaskList\n";
  out << "Number of tasks :" << fTaskList.size () << endl;
  list < OOPTaskControl * >::iterator j;
  for (j = fTaskList.begin (); j != fTaskList.end (); j++)
    out << (*j)->Id () << endl;
  out << "Print fExecutable\n";
  out << "Number of tasks :" << fExecutable.size () << endl;
  for (j = fExecutable.begin (); j != fExecutable.end (); j++)
    out << (*j)->Id () << endl;
}
void OOPTaskManager::ExtractGrantAccessFromTag(const OOPAccessTag & tag)
{
  if (!pthread_equal (fExecuteThread, pthread_self ())) {
#ifdef LOGPZ
    stringstream sout;
    sout << __PRETTY_FUNCTION__ << " called by foreign thread";
    LOGPZ_ERROR (logger, sout.str ());
#endif
  }
  list < OOPTaskControl * >::iterator i;
  bool found = false;
  for (i = fTaskList.begin (); i != fTaskList.end (); i++) {
    OOPTaskControl *tc = (*i);
    if (tc->Task ()->Id () == tag.TaskId()) {
      found = true;
      tc->Task()->GrantAccess (tag);
      {
#ifdef OOP_MPE
        stringstream auxsout;
        auxsout << "T:" << tag.TaskId() << " " << tag.AccessMode();
        OOPSoloEvent solo("grantaccess", auxsout.str());
#endif
#ifdef LOGPZ
	stringstream sout;
	sout << "Access Granted to taskId " << tag.TaskId() << " classid " << tc->Task()->ClassId()
	 << " on data " << tag.Id ();
	LOGPZ_DEBUG (tasklogger, sout.str ());
#endif
      }
      if (tc->Task()->CanExecute ()) {
#ifdef LOGPZ
	stringstream sout;
	sout << "Task " << tag.TaskId() << " classid " << tc->ClassId ();
	sout << " can execute";
	LOGPZ_DEBUG (tasklogger, sout.str ());
#endif
	TransfertoExecutable (tc->Task ()->Id ());
	{
#ifdef LOGPZ
	  stringstream sout;
	  sout << "OOPTaskManager task is executable " << tag.TaskId() <<
	    " classid " << tc->ClassId ();
	  LOGPZ_DEBUG (tasklogger, sout.str ());
#endif
	}
      }
      break;
    }
  }
  if (!found) {
#ifdef LOGPZ
    stringstream sout;
    sout << "Task not found on current TM";
    sout << "Task ";
    tag.TaskId().Print(sout);
    LOGPZ_ERROR (tasklogger, sout.str ());
#endif
  }
  
}
void OOPTaskManager::ExtractCancelTaskFromTag(const OOPAccessTag & tag)
{
#warning  "Implement me"
} 

void OOPTaskManager::HandleMessages()
{
  std::list< std::pair<int, OOPAccessTag> > tempList;
  {
    OOPTMLock lock;
    tempList = fMessages;
    fMessages.clear();
  }
  std::list< std::pair<int, OOPAccessTag> >::iterator it;
  it = tempList.begin();
  for(it = tempList.begin();it != tempList.end();it++)
  {
    switch(it->first)
    {
      case ETMAccessGranted:
      {
#ifdef LOG4CXX
        std::stringstream sout;
        sout << "Access Granted according to Tag ";
        it->second.ShortPrint(sout);
        LOGPZ_DEBUG(logger,sout.str());
#endif
        ExtractGrantAccessFromTag(it->second);
      }
      break;
      case ETMCancelTask:
      {
#ifdef LOG4CXX
        std::stringstream sout;
        sout << "Extract CancelTask according to Tag ";
        it->second.ShortPrint(sout);
        LOGPZ_DEBUG(logger,sout.str());
#endif
        ExtractCancelTaskFromTag(it->second); 
      }
      break;
      default:
      {
#ifdef LOGPZ    
        stringstream sout;
        sout << "Message Submitted with wrong message type, expect trouble";
        LOGPZ_DEBUG(logger,sout.str().c_str());
#endif  
      }
    }
  }
  
}

OOPTerminationTask::~OOPTerminationTask ()
{
}

OOPTerminationTask::OOPTerminationTask (int ProcId):OOPTask (ProcId)
{
}
OOPTerminationTask::OOPTerminationTask (const OOPTerminationTask & term):
OOPTask (term)
{
}

OOPMReturnType
OOPTerminationTask::Execute ()
{
#ifdef LOGPZ
  stringstream sout;
  sout << "Executing TerminationTask on Processor " << CM->GetProcID();
  LOGPZ_DEBUG(logger, sout.str());
  cout << sout.str() << endl;
#endif
  {
  OOPTMLock lock;
  TM->SetKeepGoing (false);
  }
  {
  OOPDMLock lock;
  DM->SetKeepGoing(false);
  }
  //sleep(1);
  IncrementWriteDependentData();
  return ESuccess;
}

void
OOPTerminationTask::Write (TPZStream & buf, int withclassid)
{
  OOPTask::Write (buf, withclassid);

}

void
OOPTerminationTask::Read (TPZStream & buf, void *context)
{
  OOPTask::Read (buf, context);

}

long int
OOPTerminationTask::ExecTime ()
{
  return -1;
}

TPZSaveable *
OOPTerminationTask::Restore (TPZStream & buf, void *context)
{
  OOPTerminationTask *v = new OOPTerminationTask (0);
  v->Read (buf);
  return v;
}


extern OOPTaskManager *TM;
