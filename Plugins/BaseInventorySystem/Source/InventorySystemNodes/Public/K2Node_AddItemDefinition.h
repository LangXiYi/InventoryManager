// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_ConstructObjectFromClass.h"
#include "K2Node_AddItemDefinition.generated.h"

/**
 * todo::自定义蓝图节点，使得用户可以设置创建的物品实例类型然后将该物品添加至容器内
 */
UCLASS()
class INVENTORYSYSTEMNODES_API UK2Node_AddItemDefinition : public UK2Node_ConstructObjectFromClass
{
	GENERATED_BODY()

public:
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual FText GetKeywords() const override;

	/** Returns if the node uses Outer input */
	virtual bool UseOuter() const override { return false; }

protected:
	/** Gets the node for use in lists and menus */
	virtual FText GetBaseNodeTitle() const override;
	/** Gets the default node title when no class is selected */
	virtual FText GetDefaultNodeTitle() const override;
	/** Gets the node title when a class has been selected. */
	virtual FText GetNodeTitleFormat() const override;
	/** Gets base class to use for the 'class' pin.  UObject by default. */
	virtual UClass* GetClassPinBaseClass() const override;
};
