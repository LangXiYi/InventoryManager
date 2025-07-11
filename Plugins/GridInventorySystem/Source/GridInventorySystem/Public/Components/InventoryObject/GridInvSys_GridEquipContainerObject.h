// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Components/InventoryObject/InvSys_BaseEquipContainerObject.h"
#include "Data/GridInvSys_InventoryItemInstance.h"
#include "GridInvSys_GridEquipContainerObject.generated.h"

class UGridInvSys_InventoryItemInstance;
class UGridInvSys_InventoryComponent;
struct FGridInvSys_InventoryList;
class UGridInvSys_EquipmentSlotWidget;
class UGridInvSys_ContainerGridWidget;


class UInvSys_InventoryItemDefinition;
class UInvSys_InventoryItemInstance;
class UInvSys_PreEditInventoryObject;
class UInvSys_BaseInventoryObject;

struct FInvSys_InventoryItem;
struct FInvSys_ContainerList;


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
	UGridInvSys_GridEquipContainerObject();
	
	virtual void AddInventoryItemToEquipSlot_DEPRECATED(const FInvSys_InventoryItem& NewItem) override;
	virtual void AddInventoryItemToContainer_DEPRECATED(const FGridInvSys_InventoryItem& InventoryItem);
	virtual void RemoveInventoryItemFromContainer_DEPRECATED(FGridInvSys_InventoryItem InventoryItem);
	virtual void UpdateInventoryItemFromContainer_DEPRECATED(FName ItemUniqueID, FGridInvSys_InventoryItemPosition NewPosition);

	virtual UInvSys_EquipSlotWidget* CreateDisplayWidget(APlayerController* PC) override;
	
protected:
	void OnItemPositionChange(const FGridInvSys_ItemPositionChangeMessage& Message);
	virtual void OnInventoryStackChange(const FInvSys_InventoryStackChangeMessage& ChangeInfo) override;
	virtual void OnContainerEntryAdded(const FInvSys_ContainerEntry& Entry, bool bIsInit) override;
	virtual void OnContainerEntryRemove(const FInvSys_ContainerEntry& Entry, bool bIsInit) override;
	
private:
	int32 FindContainerItemIndex(FName ItemUniqueID);

public:
	/**
	 * Getter Or Setter
	 **/

	/** [Server] */
	bool IsValidPosition(const FGridInvSys_InventoryItemPosition& ItemPosition);

	/** [Client] */
	bool FindEnoughFreeSpace(FIntPoint ItemSize, FGridInvSys_InventoryItemPosition& OutPosition) const;
	
	bool FindContainerGridItem(const FIntPoint& ItemPosition, FGridInvSys_InventoryItem& OutItem) const;
	
	 bool FindContainerGridItem(FName ItemUniqueID, FGridInvSys_InventoryItem& OutItem) const;
	
	virtual void CopyPropertyFromPreEdit(UObject* PreEditPayLoad) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	/** 该装备槽支持装备的类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Equipment Object")
	EGridInvSys_InventoryItemType EquipmentSupportType = EGridInvSys_InventoryItemType::None;

	/** 当前容器内的所有物品 */
	TArray<FGridInvSys_InventoryItem> RepNotify_ContainerItems_DEPRECATED;
	
private:
	/** [Server] 优化物品查询速度  Key ===> Position : Value ===> ItemUniqueId */
	TMap<FIntPoint, FName> ItemPositionMap;
	/** [Client & Server] 加快物品的查询速度。 Key ===> ItemUniqueID */
	TMap<FName, FGridInvSys_InventoryItem> ContainerGridItems;
	/** 存储 UniqueID, Position，更新物品位置信息时只会修改这个？ */
	TMap<FName, FGridInvSys_InventoryItemPosition> GridItemPositions;
	/** [Client] <GridID, ContainerGridWidgets> 供客户端使用，会在创建控件之后自动填充。 */
	TMap<FName, UGridInvSys_ContainerGridWidget*> ContainerGridWidgets;
	/** [Server] 仅服务器使用，存储当前容器内的不同网格的大小 */
	TMap<FName, FIntPoint> ContainerGridSizeMap;
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

	/** 该装备槽支持装备的类型 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container Type")
	EGridInvSys_InventoryItemType EquipmentSupportType;
};