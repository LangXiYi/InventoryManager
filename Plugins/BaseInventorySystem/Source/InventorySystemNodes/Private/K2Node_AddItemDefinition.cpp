// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_AddItemDefinition.h"
#include "Components/InvSys_InventoryComponent.h"

struct FK2Node_AddItemDefinitionHelper
{
	static FName GetInventoryPinName()
	{
		return TEXT("InvComp");
	}
};

void UK2Node_AddItemDefinition::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// InventoryComponent pin
	UEdGraphPin* InventoryPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UInvSys_InventoryComponent::StaticClass(), FK2Node_AddItemDefinitionHelper::GetInventoryPinName());
}

void UK2Node_AddItemDefinition::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
	// todo::InventoryPin->AddItemInstance
}

FText UK2Node_AddItemDefinition::GetKeywords() const
{
	return NSLOCTEXT("K2Node", "AddItemDefinition", "Add Item Definition");
}

FText UK2Node_AddItemDefinition::GetBaseNodeTitle() const
{
	return NSLOCTEXT("K2Node", "AddItemDefinition_BaseTitle", "Add Item Definition");
}

FText UK2Node_AddItemDefinition::GetDefaultNodeTitle() const
{
	return NSLOCTEXT("K2Node", "AddItemDefinition_Title_NONE", "Add Item Definition NONE");
}

FText UK2Node_AddItemDefinition::GetNodeTitleFormat() const
{
	return NSLOCTEXT("K2Node", "AddItemDefinition", "AddItemDefinition {ClassName}");
}

UClass* UK2Node_AddItemDefinition::GetClassPinBaseClass() const
{
	return UObject::StaticClass();
}
