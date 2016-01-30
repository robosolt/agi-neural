// (C)opyright by Oscar ter Hofstede
// All rights reserved.
//
// Created on 23-11-2006 by Oscar ter Hofstede
//
// $Revision: $
// $Date: $
// $Author: $

#ifndef __CSPIKINGNODE_H__
#define __CSPIKINGNODE_H__

#include <list>
#include <queue>

////////////////////////////////////////////////////////////////////////////////////////////////////

class CSpikingNode;
class CSpikingNet;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Base class for links between nodes
//
//	Formerly, these were simply implemented by
//		typedef std::pair<CSpikingNode*,bool> TLink;
//	but as we want to expand functionallity, it's now a proper class
//

class TLink
{
public:
							TLink (CSpikingNode *inPeer, float inWeight) : mPeer(inPeer), mWeight(inWeight) { }

	CSpikingNode *			mPeer;
	float					mWeight;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Base class for nodes that fire when reaching some criterium
//

class CSpikingNode
{
public:

							CSpikingNode (CSpikingNet *inNet);
	virtual					~CSpikingNode (void);

	void					LinkTo (CSpikingNode &inDstNode, float inWeight);

	void					Reset (void);
	void					ResetEpochs (void);

	void					Incoming (double inStrength);

	bool					Fire (void);

	bool					HasFiredAtLeastOnce (void) { return mHasFiredAtLeastOnce; }

	double					GetExcitation (bool inNormalized = false);
	double					GetFireThreshold (void) { return mFireThreshold; }
	double					GetFireCount (void) { return mFireCount; }

protected:

	friend					CSpikingNet;

//	std::list<TLink>		mInputs;			// backward links; currently we have no use for these
	std::vector<TLink>		mOutputs;			// forward links

	// tuning parameters
	double					mDecayFactor;		// each epoch, the excitation decays by this factor
	double					mFireDecay;			// after firing, the excitation will be the threshold times this value
	double					mSensitivityFactor;	// indicates the sensitivity of the node

	// internal state parameters
	double					mFireThreshold;		// indicates the current firing threshold
	double					mFireStrength;		// indicates the strength with which to fire
	double					mLastExcitation;
	unsigned int			mLastEpoch;
	unsigned int			mFireCount;			// number of times it fired during the last reset

private:

	void					AssureExcitationIsUpToDate (void);

	bool					mIsQueuedToFire;
	bool					mHasFiredAtLeastOnce;

	CSpikingNet *			mNet;
};


////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // __CSPIKINGNODE_H__
