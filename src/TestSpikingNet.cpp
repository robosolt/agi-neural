// (C)opyright by Oscar ter Hofstede
// All rights reserved.
//
// Created on 23-11-2006 by Oscar ter Hofstede
//
// $Revision: $
// $Date: $
// $Author: $

#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include "CSpikingNet.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned long kNodeCount = 10000;
static unsigned long kMinOutputCount = 10;
static unsigned long kMaxOutputCount = 100;
static unsigned long kStimuliCount = 100;
static unsigned long kStimulatedNodesCount = 100;
static unsigned long kRunTimeInSecs = 20;

static CSpikingNet sMyNet;
static std::vector<CSpikingNode*> sMyCopy;
static std::vector<CSpikingNode*> sStimuli;

////////////////////////////////////////////////////////////////////////////////////////////////////

#define MyGetTickCount GetTickCountMilliSeconds

unsigned long 
GetTickCountMilliSeconds (void)
{
	// Helper function; returns tickcount in milliseconds units, and in millisecond precision.
	// GetTickCount() returns millisecond units, but the precision depends on system settings (usually centi-seconds)
	// Cost of GetTickCount is about 0.01 microseconds on a 3GHz P4 with HT
	// Cost of this funcion is about 0.33 microseconds on the same machine
	// Cost of this funcion is about 1.97 microseconds on the same machine if no high resolution time is present

	static bool sHasHighResTickCount = false;
	static bool sFirst = true;
	static LARGE_INTEGER sFreq;
	if (sFirst)
	{
		sHasHighResTickCount = QueryPerformanceFrequency (&sFreq) == TRUE;
		sFirst = false;
	}

	unsigned long theTickCount;

	if (sHasHighResTickCount)
	{
		LARGE_INTEGER cnt;
		bool result = QueryPerformanceCounter (&cnt) == TRUE;

		theTickCount = (unsigned long) ((1000 * cnt.QuadPart) / sFreq.QuadPart);
	}
	else
	{
		timeBeginPeriod (1);
		theTickCount = timeGetTime();
		timeEndPeriod (1);
	}

	return theTickCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////


unsigned long
RandomInt (unsigned long inMax)
{
	unsigned long LONG_RAND_MAX = (1UL + RAND_MAX) * RAND_MAX + RAND_MAX;
	double theRand = rand();
	theRand = theRand * (1UL + RAND_MAX) + rand();

	unsigned long r = (unsigned long)((theRand / LONG_RAND_MAX) * inMax);
	if (r > inMax)
		r = inMax;

	return r;
}


////////////////////////////////////////////////////////////////////////////////////////////////////


void
BuildNet (unsigned long inNodeCount)
{
	sMyCopy.resize (inNodeCount);

	for (unsigned long i = 0; i < inNodeCount; i++)
	{
		CSpikingNode *theNode = new CSpikingNode (&sMyNet);
		sMyNet.AddNode (theNode);
		sMyCopy[i] = theNode;
	}

	for (unsigned long theNodeIndex = 0; theNodeIndex < inNodeCount; theNodeIndex++)
	{
		unsigned long theOutputCount = kMinOutputCount + RandomInt (kMaxOutputCount-kMinOutputCount);

		for (unsigned long k = 0; k < theOutputCount; k++)
		{
			unsigned long theOtherNodeIndex = RandomInt (inNodeCount-1);
			CSpikingNode *theSrcNode = sMyCopy[theNodeIndex];
			CSpikingNode *theDstNode = sMyCopy[theOtherNodeIndex];
			assert (theSrcNode);
			assert (theDstNode);
			theSrcNode->LinkTo (*theDstNode, (rand() & 1) == 1 ? 1.0f : -1.0f);
		}

		if (theNodeIndex % 1000 == 0)
			printf ("\rNodes added: %ld", theNodeIndex);
	}

	printf ("\rNodes added: %ld\n", inNodeCount);
}


void
BuildStimuli (unsigned long inNodeCount)
{
	sStimuli.resize (inNodeCount);

	for (unsigned long i = 0; i < inNodeCount; i++)
	{
		CSpikingNode *theNode = new CSpikingNode (&sMyNet);

//		unsigned long theOtherNodeIndex = RandomInt (sMyCopy.size() - kStimulatedNodesCount - 1);
		for (unsigned long k = 0; k < kStimulatedNodesCount; k++)
		{
			unsigned long theOtherNodeIndex = RandomInt ((unsigned long) sMyCopy.size() - 1);
			theNode->LinkTo (*(sMyCopy[theOtherNodeIndex]), 1.0f);
		}

		sStimuli[i] = theNode;

		printf ("\rStimuli added %ld", i+1);
	}

	printf ("\n");
}


void
RunNet (void)
{
	unsigned long theStartTickCount = MyGetTickCount();
	unsigned long theStopTickCount = theStartTickCount + kRunTimeInSecs * 1000;
	unsigned long theNewTickCount = theStartTickCount;

	// fire new stimuli for the next+1 epoch
	for (unsigned long i = 0; i < sStimuli.size(); i++)
		sStimuli[i]->Incoming (2.0);
	sMyNet.StartNewEpoch();

	printf ("\n%5ld sec; Epoch %ld, %ld queued ... ", kRunTimeInSecs, 0L, (int) sMyNet.GetFiringQueueSize());

	unsigned long theActualTotalEpochs = 0;

	// Note: about 10% speed gain can be reached by removing the frequent calls to MyGetTickCount()
	for (; theNewTickCount < theStopTickCount; theNewTickCount = MyGetTickCount())
	{
		sMyNet.RunEpoch();
		theActualTotalEpochs++;

		unsigned long theEpoch = sMyNet.GetCurrentEpoch();
		unsigned long theSecsRemaining = (theStopTickCount - theNewTickCount) / 1000;
		printf ("done\n%5ld sec; Epoch %ld, %ld queued ... ", theSecsRemaining, theEpoch, (int) sMyNet.GetFiringQueueSize());

		// fire new stimuli for the next+1 epoch
		for (unsigned long i = 0; i < sStimuli.size(); i++)
		{
			sStimuli[i]->Reset();
			sStimuli[i]->Incoming (2.0);
		}

		if (sMyNet.GetCurrentEpoch() % 100 == 0)
			sMyNet.ResetEpochs();
	}

	unsigned long theEndTickCount = MyGetTickCount();

	unsigned long theActualRuntime = theEndTickCount - theStartTickCount;
	double theActualRuntimeInSecs = theActualRuntime / 1000.0;

	double theNodesPerSecond = sMyNet.GetFireCount() / theActualRuntimeInSecs;
	double theNodesPerEpoch = sMyNet.GetFireCount() / theActualTotalEpochs;
	double theEpochsPerSecond = theActualTotalEpochs / theActualRuntimeInSecs;

	unsigned long theAmountOfNodesThatFireAtLeastOnce = 0;
	for (unsigned long i = 0; i < sMyCopy.size(); i++)
	{
		if (sMyCopy[i]->HasFiredAtLeastOnce())
			theAmountOfNodesThatFireAtLeastOnce++;
	}

//	printf ("\rDone; Epoch %ld done, %7ld fired, %7ld queued           ", sMyNet.GetCurrentEpoch(), sMyNet.GetFireCount(), sMyNet.GetFiringQueueSize());
	printf ("\n\nTotal fired: %ld", sMyNet.GetFireCount());
	printf ("\nFired nodes: %ld of %ld", theAmountOfNodesThatFireAtLeastOnce, (int) sMyCopy.size());
	printf ("\nFire rate: %ld / sec", (unsigned long) theNodesPerSecond);
	printf ("\nFire rate: %ld / epoch", (unsigned long) theNodesPerEpoch);
	printf ("\nEpochrate: %.1f / sec", theEpochsPerSecond);
	printf ("\nRun time: %.1f secs", theActualRuntimeInSecs);
	printf ("\nDone.\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////


int _tmain(int argc, _TCHAR* argv[])
{
	srand(MyGetTickCount());

	BuildNet (kNodeCount);
	BuildStimuli (kStimuliCount);
	RunNet();

	printf ("\nPress enter to exit.");

	getchar();

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
