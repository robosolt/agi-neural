// (C)opyright by Oscar ter Hofstede
// All rights reserved.
//
// Created on 23-11-2006 by Oscar ter Hofstede
//
// $Revision: $
// $Date: $
// $Author: $

#include "CSpikingNet.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	This is an implementation of a spiking network. See also CSpikingNode for more details.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	The network is updated by popping nodes from a firing queue and calling the Fire() function of
//	that node. This function in turn calls the Incoming() function of each connected node, which in
//	turn updates the activation value of that node. Nodes do not fire immediately when the firing
//	threshold is reached, but are put in its turn in the firing queue of the next epoch. Thus,
//	firing is always delayed by one epoch.
//
//	Epochs are unsigned ints, which means that we can count for 49 days if the system operates at
//	1000 epochs per second and we have a 32 bit int. In order to avoid wrap-around problems, on
//	32-bit systems we can introduce a "sleep" period in which the system renormalizes (resets) all
//	the epoch counters (not implemented). On a 64-bit system we don't have any worries.
//
////////////////////////////////////////////////////////////////////////////////////////////////////


CSpikingNet::CSpikingNet (void) :
	mCurrentEpoch (0),
	mFireCount (0)
{
}


CSpikingNet::~CSpikingNet (void)
{
	std::list<CSpikingNode*>::iterator it = mNodeList.begin();
	for (; it != mNodeList.end(); it++)
		delete *it;
}

	
////////////////////////////////////////////////////////////////////////////////////////////////////


void
CSpikingNet::AddNode (CSpikingNode *inNode)
{
	mNodeList.push_back (inNode);
}


std::list<CSpikingNode*>&
CSpikingNet::GetNodeList (void)
{
	return mNodeList;
}


////////////////////////////////////////////////////////////////////////////////////////////////////


void
CSpikingNet::QueueToFire (CSpikingNode *inNode)
{
	mFiringQueue.push (inNode);
	inNode->mIsQueuedToFire = true;
}


CSpikingNode *
CSpikingNet::GetNodeFromFiringQueue (bool inPop)
{
	if (mFiringQueue.size() == 0)
		return 0;

	CSpikingNode *theNode = mFiringQueue.front();

	if (inPop)
	{
		mFiringQueue.pop();

		if (theNode == 0)
			StartNewEpoch();
	}

	return theNode;
}


size_t
CSpikingNet::GetFiringQueueSize (void)
{
	return mFiringQueue.size();
}


unsigned int
CSpikingNet::GetFireCount (void)
{
	return mFireCount;
}


////////////////////////////////////////////////////////////////////////////////////////////////////


void
CSpikingNet::SetCurrentEpoch (unsigned long inEpoch)
{
	mCurrentEpoch = inEpoch;
}


unsigned int
CSpikingNet::GetCurrentEpoch (void)
{
	return mCurrentEpoch;
}


void
CSpikingNet::StartNewEpoch (void)
{
	mCurrentEpoch++;
	mFiringQueue.push (0);	// indicates end of epoch
}


void
CSpikingNet::ResetEpochs (void)
{
	std::list<CSpikingNode*>::iterator it = mNodeList.begin();
	for (; it != mNodeList.end(); it++)
		(*it)->ResetEpochs();

	mCurrentEpoch = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////


bool
CSpikingNet::RunOne (void)
{
	CSpikingNode *theNode = GetNodeFromFiringQueue (true);

	if (theNode)
	{
		if (theNode->Fire())
			mFireCount++;
	}

	return theNode == 0;
}


void
CSpikingNet::RunEpoch (void)
{
	while (!RunOne()) { }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
