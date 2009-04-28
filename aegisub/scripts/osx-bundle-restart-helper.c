/*
  Copyright (c) 2009 Niels Martin Hansen

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the Aegisub Group nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

/*
Return codes:
1: Error in kqueue()
2: Error in kevent()
3: Timed out waiting for parent to exit
4: Error in execve()

If none of those happen the requested command is execve()'d.
*/
int main(int argc, char *argv[], char *env[])
{
	int waitpid = getppid();
	int queue, nchange;
	struct kevent event[1];
	struct kevent change[1];
	struct timespec timeout = { 30, 0 };

	if ((queue = kqueue()) == -1)
	{
		perror("kqueue()");
		return 1;
	}

	EV_SET(event,
	       waitpid,
	       EVFILT_PROC,
	       EV_ADD|EV_ENABLE|EV_ONESHOT,
	       NOTE_EXIT,
	       0, 0);

	nchange = kevent(queue, change, 1, event, 1, &timeout);
	if (nchange < 0)
	{
		perror("kevent()");
		return 2;
	}
	else if (nchange == 0)
	{
		return 3;
	}
	else if (change[0].flags & EV_ERROR)
	{
		return 2;
	}
	else
	{
		close(queue);
		if (execve(argv[1], argv+1, env) == -1)
		{
			perror("execve()");
			return 4;
		}
		return 0; /* never reached */
	}
}
