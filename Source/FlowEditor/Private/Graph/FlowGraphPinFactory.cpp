// Copyright Torbie LLC

//-----------------------------------------------------------------------------
// Torbie Begin Change
#include "Graph/FlowGraphPinFactory.h"

#include "NodeFactory.h"

#include "Graph/FlowGraphSettings.h"
#include "Nodes/FlowNode.h"
#include "Graph/Widgets/SFlowGraphNode.h"


TSharedPtr<SGraphPin> FFlowGraphPinFactory::CreatePin(
	UEdGraphPin* Pin
	) const
{
	if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec)
	{
		const TSharedPtr<SGraphPin> NewPin = SNew(SFlowGraphPinExec, Pin);

		auto* graphNode = Cast<UFlowGraphNode>(Pin->GetOuter());
		if (!UFlowGraphSettings::Get()->bShowDefaultPinNames && graphNode && graphNode->GetFlowNode())
		{
			if (Pin->Direction == EGPD_Input)
			{
				if (graphNode->GetFlowNode()->GetInputPins().Num() == 1 && Pin->PinName == UFlowNode::DefaultInputPin.PinName)
				{
					NewPin->SetShowLabel(false);
				}
			}
			else
			{
				if (graphNode->GetFlowNode()->GetOutputPins().Num() == 1 && Pin->PinName == UFlowNode::DefaultOutputPin.PinName)
				{
					NewPin->SetShowLabel(false);
				}
			}
		}

		return NewPin;
	}

	return FNodeFactory::CreateK2PinWidget(Pin);
}

// Torbie End Change
//-----------------------------------------------------------------------------
