// Copyright Torbie LLC

//-----------------------------------------------------------------------------
// Torbie Begin Change
#pragma once

#include "IDetailPropertyExtensionHandler.h"


enum class EFlowExposeAsPinType : uint8;

struct FFlowExposedParam;

class IDetailLayoutBuilder;
class UFlowGraphNode;

class FFlowAssetDetailPropertyExtension : public TSharedFromThis<FFlowAssetDetailPropertyExtension>, public IDetailPropertyExtensionHandler
{
public:
	static FName NAME_DisplayName;
	static FName NAME_ExposeAsPin;
	static FName NAME_ExposeAsPinInputOnly;
	static FName NAME_ExposeAsPinOutputOnly;

	// IDetailPropertyExtensionHandler interface
	bool IsPropertyExtendable(
		const UClass*          objectClass, 
		const IPropertyHandle& propertyHandle
		) const override;

	void ExtendWidgetRow(
		FDetailWidgetRow&           widgetRow, 
		const IDetailLayoutBuilder& detailBuilder,
		const UClass*               objectClass, 
		TSharedPtr<IPropertyHandle> propertyHandle
		) override;

protected:
	UFlowGraphNode* FindOwnerGraphNode(
		TSharedPtr<IPropertyHandle> propertyHandle
		) const;

	FFlowExposedParam* FindExposedParam(
		TSharedPtr<IPropertyHandle> propertyHandle
		) const;

	FText GetExposeAsPinFieldText(
		TSharedPtr<IPropertyHandle> propertyHandle
		) const;

	void SetExposeAsPin(
		TSharedPtr<IPropertyHandle> propertyHandle,
		EFlowExposeAsPinType exposeAsPinType
		);

	bool IsExposeAsPinSelected(
		TSharedPtr<IPropertyHandle> propertyHandle,
		EFlowExposeAsPinType exposeAsPinType
		) const;

	EVisibility GetPropertyValueVisibility(
		TSharedPtr<IPropertyHandle> propertyHandle
		) const;

	bool IsPropertyValueEnabled(
		TSharedPtr<IPropertyHandle> propertyHandle
		) const;
};

// Torbie End Change
//-----------------------------------------------------------------------------
