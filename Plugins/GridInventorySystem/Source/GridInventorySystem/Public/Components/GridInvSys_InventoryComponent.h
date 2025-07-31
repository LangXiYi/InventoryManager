// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/InvSys_InventoryComponent.h"
#include "GridInvSys_InventoryComponent.generated.h"


class UGridInvSys_InventoryItemInstance;
class UGridInvSys_ContainerGridWidget;
class UGridInvSys_DragItemWidget;
class UInvSys_InventoryItemDefinition;
class UGridInvSys_InventoryItemInfo;
/***
 * GridItem ----> ContainerGrid ----> InventoryContainer ----> InventoryComponent
 * Item --------> Grid--------------> Container -------------> Inventory
 * 库存组件可以由单个或多个容器组成，每个容器包含单个或多个网格系统，网格系统则是由网格单元组成
 */
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventoryComponent : public UInvSys_InventoryComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGridInvSys_InventoryComponent();

	// 可根据项目需要资源扩展该函数
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddItemDefinitionToContainerPos(TSubclassOf<UInvSys_InventoryItemDefinition> ItemDef, int32 StackCount, FGridInvSys_ItemPosition Pos);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddItemInstanceToContainerPos(UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RestoreItemInstanceToPos(UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);

	UFUNCTION(BlueprintCallable)
	bool FindEmptyPosition(UInvSys_InventoryItemInstance* InItemInstance, FGridInvSys_ItemPosition& OutPosition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UpdateItemInstancePosition(UInvSys_InventoryItemInstance* ItemInstance, FGridInvSys_ItemPosition NewPosition);

public:
	/**
	 * Getter Or Setter
	 **/

	UFUNCTION(BlueprintCallable, Category = "Grid Inventory Component")
	UGridInvSys_ContainerGridWidget* FindContainerGridWidget(FGameplayTag SlotTag, int32 GridID);

	UGridInvSys_ContainerGridWidget* FindContainerGridWidget(UGridInvSys_InventoryItemInstance* InItemInstance);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Grid Inventory Component")
	TArray<FGameplayTag> DefaultContainerPriority;
};
