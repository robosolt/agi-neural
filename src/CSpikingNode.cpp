// (C)opyright by Oscar ter Hofstede
// All rights reserved.
//
// Created on 23-11-2006 by Oscar ter Hofstede
//
// $Revision: $
// $Date: $
// $Author: $

#include "CSpikingNode.h"
#include "CSpikingNet.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	This is an implementation of a spiking neuron model. There is no learning rule implemented,
//	other than that the sensitivity of the node decreases after firing (fire threshold increases).
//
//	Synaptics weights can be set for each link between nodes, which can be positive (excitatory)
//	or negative (inhibitory).
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	If a node is bound to fire, it is put in the networks fire list. This assures a fair way to
//	fire the nodes, as opposed to a direct cascading firing effect, in which case starvation
//	may occur due to loops.
//
//	A node will not fire until it is first in the list. And only then, the activation values will
//	be reset. This avoids that a node is put in the list twice. On the other hand, this prohibits
//	also that some nodes fire more often.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Epochs are unsigned ints, which means that we can count for 49 days if the system operates at
//	1000 epochs per second and we have a 32 bit int. In order to avoid wrap-around problems, on
//	32-bit systems we can introduce a "sleep" period in which the system renormalizes (resets) all
//	the epoch counters (not implemented). On a 64-bit system we don't have any worries.
//
//	The network is updated by popping nodes using the function GetNodeFromFiringQueue() and calling
//	their Fire() function.
//
////////////////////////////////////////////////////////////////////////////////////////////////////


CSpikingNode::CSpikingNode (CSpikingNet *inNet) :
	mLastExcitation (0.0),
	mLastEpoch (0),
	mFireCount (0),
	mFireThreshold (0),
	mDecayFactor (0.99),				// after epoch:  mLastExcitation *= mDecayFactor;
	mFireDecay (0.2),					// after firing: mLastExcitation = mFireThreshold * mFireDecay
	mSensitivityFactor (1.001),			// after firing: mFireThreshold *= mSensitivityFactor
	mFireStrength (0),					// when reaching firing threshold: mFireStrength = mLastExcitation - mFireThreshold
	mIsQueuedToFire (false),
	mHasFiredAtLeastOnce (false),
	mNet (inNet)
{
	Reset();
}


CSpikingNode::~CSpikingNode (void)
{
}

	
////////////////////////////////////////////////////////////////////////////////////////////////////


void
CSpikingNode::Reset (void)
{
	mLastEpoch = mNet->GetCurrentEpoch();
	mLastExcitation = 0.0;
	mFireThreshold = 1.0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////


void
CSpikingNode::LinkTo (CSpikingNode &inDstNode, float inWeight)
{
	GetExcitation();
	inDstNode.GetExcitation();

	mOutputs.push_back (TLink(&inDstNode,inWeight));
//	inDstNode->mInputs.push_back (inSrcNode);
}


////////////////////////////////////////////////////////////////////////////////////////////////////

	
double
CSpikingNode::GetExcitation (bool inNormalized)
{
	AssureExcitationIsUpToDate();

	if (inNormalized)
	    return mLastExcitation / mFireThreshold;
	else
	    return mLastExcitation;
}


void
CSpikingNode::AssureExcitationIsUpToDate (void)
{
	if (mNet->GetCurrentEpoch() > mLastEpoch)
	{
		unsigned int theElapsedEpochs = mNet->GetCurrentEpoch() - mLastEpoch;

		// logarithmic decay
		mLastExcitation *= pow (mDecayFactor, (double)theElapsedEpochs);
		mLastEpoch = mNet->GetCurrentEpoch();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////


void
CSpikingNode::ResetEpochs (void)
{
	AssureExcitationIsUpToDate();

	mLastEpoch = 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////


void
CSpikingNode::Incoming (double inStrength)
{
	AssureExcitationIsUpToDate();

	mLastExcitation += log (1.0 + fabs(inStrength));

	if (mLastExcitation < 0)
		mLastExcitation = 0;

	bool shouldFire = (mLastExcitation > mFireThreshold);

	if (shouldFire && !mIsQueuedToFire)
	{
		mFireStrength = mLastExcitation - mFireThreshold;

		mNet->QueueToFire (this);
	}
}


bool
CSpikingNode::Fire (void)
{
	// reassure that we should fire, since we might have received negative excitations
	if (mLastExcitation <= mFireThreshold)
		return false; // we did not fire

	//	loop over all output connections and call their Incoming() function
#if 0 // if mOutputs is std::list
	std::list<TLink>::iterator it = mOutputs.begin();
	for (; it != mOutputs.end(); it++)
	{
		it->mPeer->Incoming (it->mWeight * mFireStrength);
	}
#else // if mOutputs is std::vector
	for (unsigned int i = 0; i < mOutputs.size(); i++)
		mOutputs[i].mPeer->Incoming (mOutputs[i].mWeight * mFireStrength);
#endif

	mLastExcitation = mFireThreshold * mFireDecay;
	mFireThreshold *= mSensitivityFactor;
	mIsQueuedToFire = false;
	mHasFiredAtLeastOnce = true;
	mFireCount++;

	return true; // we fired
}


////////////////////////////////////////////////////////////////////////////////////////////////////
