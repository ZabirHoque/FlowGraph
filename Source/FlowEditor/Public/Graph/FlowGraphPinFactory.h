// Copyright Torbie LLC

//-----------------------------------------------------------------------------
// Torbie Begin Change
#pragma once

#include "EdGraphUtilities.h"


class FFlowGraphPinFactory : public FGraphPanelPinFactory
{
public:
	TSharedPtr<SGraphPin> CreatePin(
		UEdGraphPin* InPin
		) const override;
};

// Torbie End Change
//-----------------------------------------------------------------------------
