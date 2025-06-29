// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/InventoryObject/InvSys_BaseEquipContainerObject.h"
#include "GridInvSys_GridEquipContainerObject.generated.h"

class UGridInvSys_EquipmentSlotWidget;
class UGridInvSys_ContainerGridWidget;
/**
 * 装备容器继承自装备对象，和装备对象一样会在开始前为库存组件添加一个可供物品装备的插槽，但是在装备物品后会根据物品的内容，创建一个新的
 * 容器，并管理该容器。
 * ============
 * 可装备的容器
 * ============
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_GridEquipContainerObject : public UInvSys_BaseEquipContainerObject
{
	GENERATED_BODY()

public:
	virtual void TryRefreshOccupant() override;

	virtual void TryRefreshContainerItems();

	virtual void CreateDisplayWidget(APlayerController* PC) override;

	virtual void AddInventoryItemToEquipSlot(const FInvSys_InventoryItem& NewItem, FName TargetSlotName) override;

	virtual void AddInventoryItemToContainer(const FGridInvSys_InventoryItem& InventoryItem);

	virtual void RemoveInventoryItemFromContainer(FGridInvSys_InventoryItem InventoryItem);

	virtual void UpdateInventoryItemFromContainer(FGridInvSys_InventoryItem NewItem, FGridInvSys_InventoryItem OldItem);

	virtual void CopyPropertyFromPreEdit(UInvSys_InventoryComponent* NewInventoryComponent, UObject* PreEditPayLoad) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	virtual void OnAddedContainerItems(const TArray<FName>& InAddedItems) override;
	virtual void OnRemovedContainerItems(const TArray<FName>& InRemovedItems) override;
	virtual void OnUpdatedContainerItems(const TArray<FName>& InChangedItems) override;

private:
	int32 FindContainerItemIndex(FName ItemUniqueID);
	
protected:
	/** 装备控件类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Equipment Object")
	TSubclassOf<UGridInvSys_EquipmentSlotWidget> EquipmentSlotWidgetClass;

	/** 装备控件 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Equipment Object")
	TObjectPtr<UGridInvSys_EquipmentSlotWidget> EquipmentSlotWidget;

	/** 该装备槽支持装备的类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Equipment Object")
	EGridInvSys_InventoryItemType EquipmentSupportType;

	/** 当前容器内的所有物品 */
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Inventory Component")
	TArray<FGridInvSys_InventoryItem> ContainerItems;

private:
	/** 加快物品的查询速度。 Key ===> ItemUniqueID */
	TMap<FName, FGridInvSys_InventoryItem> ContainerItemMap;
	
	// <GridID, ContainerGridWidgets> 供客户端使用
	TMap<FName, UGridInvSys_ContainerGridWidget*> ContainerGridMap;
};

/**
 * Pre Edit Inventory Object
 */

UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_PreEditGridEquipContainerObject : public UInvSys_PreEditEquipmentObject
{
	GENERATED_BODY()

public:
	CONSTRUCT_INVENTORY_OBJECT(UGridInvSys_GridEquipContainerObject);

	/** 装备控件类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Type")
	TSubclassOf<UUserWidget> EquipmentSlotWidgetClass;

	/** 该装备槽支持装备的类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Type")
	EGridInvSys_InventoryItemType EquipmentSupportType;
};