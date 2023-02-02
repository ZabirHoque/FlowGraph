// Copyright Torbie LLC

//-----------------------------------------------------------------------------
// Torbie Begin Change
#include "DetailCustomizations/FlowAssetDetailPropertyExtension.h"

#include "DetailLayoutBuilder.h"

#include "Nodes/FlowNode.h"
#include "Graph/Nodes/FlowGraphNode.h"


#define LOCTEXT_NAMESPACE "FlowAssetEditor"

FName FFlowAssetDetailPropertyExtension::NAME_DisplayName			= "DisplayName";
FName FFlowAssetDetailPropertyExtension::NAME_ExposeAsPin			= "ExposeAsPin";
FName FFlowAssetDetailPropertyExtension::NAME_ExposeAsPinInputOnly	= "ExposeAsPinInputOnly";
FName FFlowAssetDetailPropertyExtension::NAME_ExposeAsPinOutputOnly = "ExposeAsPinOutputOnly";

bool FFlowAssetDetailPropertyExtension::IsPropertyExtendable(
	const UClass*          objectClass,
	const IPropertyHandle& propertyHandle
	) const
{
	FProperty* property = propertyHandle.GetProperty();
	if (!property)
	{
		return false;
	}

	UStruct* ownerStruct = property->GetOwnerStruct();
	if (!ownerStruct)
	{
		return false;
	}

	if (!ownerStruct->IsChildOf<UFlowNode>())
	{
		return false;
	}

	if (!propertyHandle.HasMetaData(NAME_ExposeAsPin) &&
		!propertyHandle.HasMetaData(NAME_ExposeAsPinInputOnly) &&
		!propertyHandle.HasMetaData(NAME_ExposeAsPinOutputOnly) &&
		 ownerStruct->IsNative())
	{
		return false;
	}

	return true;
}

void FFlowAssetDetailPropertyExtension::ExtendWidgetRow(
	FDetailWidgetRow&           widgetRow,
	const IDetailLayoutBuilder& detailBuilder,
	const UClass*               objectClass,
	TSharedPtr<IPropertyHandle> propertyHandle
	)
{
	FProperty* property = propertyHandle->GetProperty();
	if (!property)
	{
		return;
	}

	UStruct* ownerStruct = property->GetOwnerStruct();
	if (!ownerStruct)
	{
		return;
	}

	constexpr bool bCloseWindowAfterSelection = true;
	FMenuBuilder menuBuilder(bCloseWindowAfterSelection, nullptr, nullptr);
	{
		bool bExposeAsPin    = propertyHandle->HasMetaData(NAME_ExposeAsPin) || !ownerStruct->IsNative();
		bool bExposeAsInput  = bExposeAsPin || propertyHandle->HasMetaData(NAME_ExposeAsPinInputOnly);
		bool bExposeAsOutput = bExposeAsPin || propertyHandle->HasMetaData(NAME_ExposeAsPinOutputOnly);

		menuBuilder.AddMenuEntry(
			LOCTEXT("ExposeAsPinType_NoneLabel", "Variable"),
			LOCTEXT("ExposeAsPinType_NoneLabel_ToolTip", "Do not expose as pin."),
			FSlateIcon(),
			FUIAction{
				FExecuteAction::CreateSP(this, &FFlowAssetDetailPropertyExtension::SetExposeAsPin, propertyHandle, EFlowExposeAsPinType::None),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &FFlowAssetDetailPropertyExtension::IsExposeAsPinSelected, propertyHandle, EFlowExposeAsPinType::None)
				},
			NAME_None,
			EUserInterfaceActionType::RadioButton
			);
		
		menuBuilder.AddMenuEntry(
			LOCTEXT("ExposeAsPinType_InLabel", "Pin (Input)"),
			LOCTEXT("ExposeAsPinType_InLabel_ToolTip", "Expose as input pin only."),
			FSlateIcon(),
			FUIAction{
				FExecuteAction::CreateSP(this, &FFlowAssetDetailPropertyExtension::SetExposeAsPin, propertyHandle, EFlowExposeAsPinType::In),
				FCanExecuteAction::CreateLambda([bExposeAsInput](){ return bExposeAsInput; }),
				FIsActionChecked::CreateSP(this, &FFlowAssetDetailPropertyExtension::IsExposeAsPinSelected, propertyHandle, EFlowExposeAsPinType::In)
				},
			NAME_None,
			EUserInterfaceActionType::RadioButton
			);

		menuBuilder.AddMenuEntry(
			LOCTEXT("ExposeAsPinType_OutLabel", "Pin (Output)"),
			LOCTEXT("ExposeAsPinType_OutLabel_ToolTip", "Expose as output pin only."),
			FSlateIcon(),
			FUIAction{
				FExecuteAction::CreateSP(this, &FFlowAssetDetailPropertyExtension::SetExposeAsPin, propertyHandle, EFlowExposeAsPinType::Out),
				FCanExecuteAction::CreateLambda([bExposeAsOutput]() { return bExposeAsOutput; }),
				FIsActionChecked::CreateSP(this, &FFlowAssetDetailPropertyExtension::IsExposeAsPinSelected, propertyHandle, EFlowExposeAsPinType::Out)
				},
			NAME_None,
			EUserInterfaceActionType::RadioButton
			);

		menuBuilder.AddMenuEntry(
			LOCTEXT("ExposeAsPinType_InOutLabel", "Pin (Input/Output)"),
			LOCTEXT("ExposeAsPinType_InOutLabel_ToolTip", "Expose as input & output pin."),
			FSlateIcon(),
			FUIAction{
				FExecuteAction::CreateSP(this, &FFlowAssetDetailPropertyExtension::SetExposeAsPin, propertyHandle, EFlowExposeAsPinType::InOut),
				FCanExecuteAction::CreateLambda([bExposeAsPin]() { return bExposeAsPin; }),
				FIsActionChecked::CreateSP(this, &FFlowAssetDetailPropertyExtension::IsExposeAsPinSelected, propertyHandle, EFlowExposeAsPinType::InOut)
				},
			NAME_None,
			EUserInterfaceActionType::RadioButton
			);
	}

	TSharedRef<SWidget> defaultValueWidget = widgetRow.ValueContent().Widget;

	widgetRow.ValueContent()
	[
		SNew(SBox)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 0.0f))
		.IsEnabled(this, &FFlowAssetDetailPropertyExtension::IsPropertyValueEnabled, propertyHandle)
		[
			defaultValueWidget
		]
	];

	widgetRow.ExtensionContent()
	[
		SNew(SComboButton)
		.ContentPadding(0)
		.MenuContent()
		[
			menuBuilder.MakeWidget()
		]
		.ButtonContent()
		[
			SNew(SBox)
			.Padding(FMargin(0.0f, 0.0f, 2.0f, 0.0f))
			.MinDesiredWidth(50.f)
			[
				SNew(STextBlock)
				.Text(this, &FFlowAssetDetailPropertyExtension::GetExposeAsPinFieldText, propertyHandle)
				.Font(detailBuilder.GetDetailFont())
			]
		]
	];
}

UFlowGraphNode* FFlowAssetDetailPropertyExtension::FindOwnerGraphNode(
	TSharedPtr<IPropertyHandle> propertyHandle
	) const
{
	if (!propertyHandle)
	{
		return nullptr;
	}

	FProperty* property = propertyHandle->GetProperty();
	if (!property)
	{
		return nullptr;
	}

	TArray<UObject*> ownerObjects;
	propertyHandle->GetOuterObjects(ownerObjects);
	if (ownerObjects.Num() != 1)
	{
		return nullptr;
	}
	
	auto* ownerNode = Cast<UFlowNode>(ownerObjects[0]);
	if (!ownerNode)
	{
		return nullptr;
	}

	auto* parentGraphNode = Cast<UFlowGraphNode>(ownerNode->GetGraphNode());
	return parentGraphNode;
}

FFlowExposedParam* FFlowAssetDetailPropertyExtension::FindExposedParam(
	TSharedPtr<IPropertyHandle> propertyHandle
	) const
{
	if (!propertyHandle)
	{
		return nullptr;
	}

	UFlowGraphNode* ownerGraphNode = FindOwnerGraphNode(propertyHandle);
	if (!ownerGraphNode) 
	{
		return nullptr;
	}

	FProperty* property = propertyHandle->GetProperty();
	if (!property)
	{
		return nullptr;
	}

	FFlowExposedParam* foundExposedParam = Algo::FindBy(
		ownerGraphNode->ExposedParams,
		property->GetFName(),
		&FFlowExposedParam::Name
		);

	return foundExposedParam;
}

FText FFlowAssetDetailPropertyExtension::GetExposeAsPinFieldText(
	TSharedPtr<IPropertyHandle> propertyHandle
	) const
{
	static FText TEXT_None	= LOCTEXT("SelectedExposeAsPinType_None",  "Variable");
	static FText TEXT_In	= LOCTEXT("SelectedExposeAsPinType_In",	   "Pin (Input)");
	static FText TEXT_Out	= LOCTEXT("SelectedExposeAsPinType_Out",   "Pin (Output)");
	static FText TEXT_InOut = LOCTEXT("SelectedExposeAsPinType_InOut", "Pin (In/Out)");

	if (!propertyHandle)
	{
		return TEXT_None;
	}

	FFlowExposedParam* foundExposedParam = FindExposedParam(propertyHandle);
	if (!foundExposedParam)
	{
		return TEXT_None;
	}

	switch (foundExposedParam->ExposeAsPinType)
	{
	default:
	case EFlowExposeAsPinType::None:
		return TEXT_None;

	case EFlowExposeAsPinType::In:
		return TEXT_In;

	case EFlowExposeAsPinType::Out:
		return TEXT_Out;

	case EFlowExposeAsPinType::InOut:
		return TEXT_InOut;
	}
}

void FFlowAssetDetailPropertyExtension::SetExposeAsPin(
	TSharedPtr<IPropertyHandle> propertyHandle,
	EFlowExposeAsPinType exposeAsPinType
	)
{
	if (!propertyHandle)
	{
		return;
	}

	UFlowGraphNode* ownerGraphNode = FindOwnerGraphNode(propertyHandle);
	if (!ownerGraphNode)
	{
		return;
	}

	FProperty* property = propertyHandle->GetProperty();
	if (!property)
	{
		return;
	}

	FName propertyName = property->GetFName();

	if (exposeAsPinType == EFlowExposeAsPinType::None)
	{
		ownerGraphNode->ExposedParams.RemoveAllSwap(
			[propertyName](const FFlowExposedParam& exposedParam)
			{
				return propertyName == exposedParam.Name;
			});
	}
	else
	{
		FFlowExposedParam* foundExposedParam = Algo::FindBy(
			ownerGraphNode->ExposedParams,
			propertyName,
			&FFlowExposedParam::Name
			);

		if (foundExposedParam)
		{
			foundExposedParam->ExposeAsPinType = exposeAsPinType;
		}
		else
		{
			int32 propertyOffset = property->GetOffset_ForInternal();

			int32 upperBoundIdx = Algo::UpperBoundBy(ownerGraphNode->ExposedParams, propertyOffset, &FFlowExposedParam::Offset);

			FFlowExposedParam& addedExposedParam = ownerGraphNode->ExposedParams.EmplaceAt_GetRef(
				upperBoundIdx,
				propertyName,
				exposeAsPinType,
				propertyOffset
				);

			addedExposedParam.ToolTip = property->GetToolTipText().ToString();

			if (property->HasMetaData(NAME_DisplayName))
			{
				addedExposedParam.FriendlyName = FText::FromString(property->GetMetaData(NAME_DisplayName));
			}
		}
	}

	ownerGraphNode->ReconstructNode();
	ownerGraphNode->GetGraph()->NotifyGraphChanged();
}

bool FFlowAssetDetailPropertyExtension::IsExposeAsPinSelected(
	TSharedPtr<IPropertyHandle> propertyHandle,
	EFlowExposeAsPinType exposeAsPinType
	) const
{
	if (!propertyHandle)
	{
		return exposeAsPinType == EFlowExposeAsPinType::None;
	}

	FFlowExposedParam* foundExposedParam = FindExposedParam(propertyHandle);
	if (!foundExposedParam)
	{
		return exposeAsPinType == EFlowExposeAsPinType::None;
	}

	return exposeAsPinType == foundExposedParam->ExposeAsPinType;
}

EVisibility FFlowAssetDetailPropertyExtension::GetPropertyValueVisibility(
	TSharedPtr<IPropertyHandle> propertyHandle
	) const
{
	if (!propertyHandle)
	{
		return EVisibility::SelfHitTestInvisible;
	}

	FFlowExposedParam* foundExposedParam = FindExposedParam(propertyHandle);
	if (!foundExposedParam)
	{
		return EVisibility::SelfHitTestInvisible;
	}

	return foundExposedParam->ExposeAsPinType == EFlowExposeAsPinType::None ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
}

bool FFlowAssetDetailPropertyExtension::IsPropertyValueEnabled(
	TSharedPtr<IPropertyHandle> propertyHandle
	) const
{
	if (!propertyHandle)
	{
		return true;
	}

	FFlowExposedParam* foundExposedParam = FindExposedParam(propertyHandle);
	if (!foundExposedParam)
	{
		return true;
	}

	return
		foundExposedParam->ExposeAsPinType == EFlowExposeAsPinType::None || 
		foundExposedParam->ExposeAsPinType == EFlowExposeAsPinType::Out;
}

#undef LOCTEXT_NAMESPACE

// Torbie End Change
//-----------------------------------------------------------------------------
