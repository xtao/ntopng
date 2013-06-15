/*
 *
 * (C) 2013 - ntop.org
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifdef WIN32

#include "ntop_includes.h"

/* **************************************

   WIN32 MULTITHREAD STUFF
   
   ************************************** */

pthread_t pthread_self(void) { return(0); }

int pthread_create(pthread_t *threadId, void* notUsed, void *(*__start_routine) (void *), void* userParm) {
  DWORD dwThreadId, dwThrdParam = 1;
  
  (*threadId) = CreateThread(NULL, /* no security attributes */
			     0,            /* use default stack size */
			     (LPTHREAD_START_ROUTINE)__start_routine, /* thread function */
			     userParm,     /* argument to thread function */
			     0,            /* use default creation flags */
			     &dwThreadId); /* returns the thread identifier */

  if(*threadId != NULL)
    return(1);
  else
    return(0);
}

/* ************************************ */

void pthread_detach(pthread_t *threadId) {
  CloseHandle((HANDLE)*threadId);
}

/* ************************************ */

int pthread_join (pthread_t threadId, void **_value_ptr) {
  int rc = WaitForSingleObject(threadId, INFINITE);
  CloseHandle(threadId);
  return(rc);
}

/* ************************************ */

#endif /* WIN32 */
