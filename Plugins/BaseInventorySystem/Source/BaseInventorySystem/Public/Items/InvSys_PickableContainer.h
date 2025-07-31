// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_PickableItems.h"
#include "InvSys_PickableContainer.generated.h"

class UInvSys_InventoryLayoutWidget;
class UInvSys_InventoryComponent;

UCLASS()
class BASEINVENTORYSYSTEM_API AInvSys_PickableContainer : public AInvSys_PickableItems
{
	GENERATED_BODY()

public:
	AInvSys_PickableContainer();

	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void SetupInventoryComponent();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Pickable Container")
	TObjectPtr<UInvSys_InventoryComponent> InventoryComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pickable Container")
	TSubclassOf<UInvSys_InventoryComponent> InventoryComponentClass;

	/** [CDO]库存内容映射，使用数据资产方便管理 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pickable Container")
	TSoftClassPtr<class UInvSys_InventoryContentMapping> InventoryObjectContent;

	/** 库存组件的展示菜单 */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Pickable Container")
	TSubclassOf<UInvSys_InventoryLayoutWidget> LayoutWidgetClass;
};
