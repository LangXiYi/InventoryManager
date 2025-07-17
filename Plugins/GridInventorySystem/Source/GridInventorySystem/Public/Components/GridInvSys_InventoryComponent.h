// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/InvSys_InventoryComponent.h"
#include "GridInvSys_InventoryComponent.generated.h"


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
	
	UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction))
	virtual void AddInventoryItemToGridContainer(FGridInvSys_InventoryItem GridContainerItem);

	/**
	 * 仅适用于同容器组件下的物品转移
	 * @param ChangedItems 旧物品的唯一ID
	 * @param NewItemData 
	 */
	UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction))
	virtual void UpdateContainerItemsPosition(TArray<FName> ChangedItems, TArray<FGridInvSys_InventoryItemPosition> NewItemData);

	/**
	 * 使用与不同容器组件下的物品转移
	 * @param ToInvCom 目标的库存组件
	 * @param FromItemData 本组件需要传给目标组件的数据
	 * @param ToItemData 目标组件需要传给本组件的数据
	 */
	UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction))
	virtual void UpdateOtherContainerItemsPosition(UGridInvSys_InventoryComponent* ToInvCom,
		TArray<FGridInvSys_InventoryItem> FromItemData, TArray<FGridInvSys_InventoryItem> ToItemData);

	UFUNCTION(BlueprintCallable, meta = (DeprecatedFunction))
	bool FindEnoughFreeSpace(FName SlotName, FIntPoint ItemSize, FGridInvSys_InventoryItemPosition& OutPosition) const;

public:
	/**
	 * RPC Function
	 **/

	bool TryDropItemInstanceToPos(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_TryDropItemInstanceToPos(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);
	
	void LocalPre_TryDropItemInstance(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_AddItemInstancesToContainerPos(UInvSys_InventoryComponent* InvComp,
		const TArray<UInvSys_InventoryItemInstance*>& InItemInstances, const TArray<FGridInvSys_ItemPosition>& InPosArray);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_RestoreItemInstanceToPos(UInvSys_InventoryComponent* InvComp,
		UInvSys_InventoryItemInstance* InItemInstance, const FGridInvSys_ItemPosition& InPos);

	
public:
	/**
	 * Getter Or Setter
	 **/

	/** 根据位置查找物品信息。 */
	bool FindInventoryItem(FName SlotName, const FIntPoint& ItemPosition, FGridInvSys_InventoryItem& OutItem);

	bool FindContainerGridItem(FName ItemUniqueID,  FGridInvSys_InventoryItem& OutItem);
	
	UUserWidget* GetInventoryLayoutWidget() const;

	/** 获取所有容器的SlotName */
	UFUNCTION(BlueprintCallable)
	void GetAllContainerSlotName(TArray<FName>& OutArray) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
