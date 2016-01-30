// (C)opyright by Oscar ter Hofstede
// All rights reserved.
//
// Created on 23-11-2006 by Oscar ter Hofstede
//
// $Revision: $
// $Date: $
// $Author: $

#ifndef __CSPIKINGNET_H__
#define __CSPIKINGNET_H__

#include "CSpikingNode.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Spiking Neural Net
//

class CSpikingNet
{
public:

								CSpikingNet (void);
	virtual						~CSpikingNet (void);

	// construction
	void						AddNode (CSpikingNode *inNode);
	std::list<CSpikingNode*>&	GetNodeList (void);

	// running
	bool						RunOne (void);
	void						RunEpoch (void);
	void						QueueToFire (CSpikingNode *inNode);

	// epochs
	void						StartNewEpoch (void);
	unsigned int				GetCurrentEpoch (void);
	void						ResetEpochs (void);

	// statistics
	unsigned int				GetFireCount (void);
	size_t						GetFiringQueueSize (void);

protected:

	CSpikingNode *				GetNodeFromFiringQueue (bool inPop);
	void						SetCurrentEpoch (unsigned long inEpoch);

	unsigned int				mCurrentEpoch;
	std::queue<CSpikingNode*>	mFiringQueue;
	unsigned int				mFireCount;

private:

	std::list<CSpikingNode*>	mNodeList;
};


////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // __CSPIKINGNET_H__
