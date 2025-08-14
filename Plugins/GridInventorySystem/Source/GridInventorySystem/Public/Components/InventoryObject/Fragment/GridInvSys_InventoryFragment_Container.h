// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/InventoryObject/Fragment/InvSys_InventoryFragment_Container.h"
#include "GridInvSys_InventoryFragment_Container.generated.h"

class UGridInvSys_ContainerGridWidget;

UENUM()
enum class EGridInvSys_ContainerLayoutDataType
{
	FromEquipment = 0,
	FromWidget,
	Custom,
};

class UGridInvSys_ContainerGridLayoutWidget;
class UGridInvSys_InventoryItemInstance;

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_InventoryFragment_Container : public UInvSys_InventoryFragment_Container
{
	GENERATED_BODY()

public:
	virtual void InitInventoryFragment(UObject* PreEditFragment) override;

	bool UpdateItemInstancePosition(UGridInvSys_InventoryItemInstance* GridItemInstance, const FGridInvSys_ItemPosition& NewPosition);

	/** 在方形范围内是否未被占据 */
	bool IsUnoccupiedInSquareRange(int32 ToGridID, FIntPoint ToPosition, FIntPoint ItemSize);

	TArray<int32> GetItemGridOccupiedIndexes(UGridInvSys_InventoryItemInstance* ItemInstance) const;

	/**
	 * 检查目标位置是否能够容纳指定的物品，若该物品在当前容器内存在则该物品所在位置视为未被占据。
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Container Fragment", meta = (AdvancedDisplay = bIsIgnoreInItemInstance))
	bool CheckItemPosition(UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& NewPosition, bool bIsIgnoreInItemInstance = true);

	void UpdateContainerGridItemState(UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& ItemPosition, bool IsOccupy);

	void UpdateContainerGridOccupyState(FIntPoint ItemSize, const FGridInvSys_ItemPosition& ItemPosition, bool IsOccupy);

	bool FindEmptyPosition(FIntPoint ItemSize, FGridInvSys_ItemPosition& OutPosition);

	// virtual void RemoveAllItemInstance() override;

protected:
	virtual void OnContainerPostAdd(UInvSys_InventoryItemInstance* ItemInstance) override;
	virtual void OnContainerPreRemove(UInvSys_InventoryItemInstance* ItemInstance) override;
	
private:
	void UpdateContainerData_FromEquip();

	void UpdateContainerData_FromWidget();
	
	void UpdateContainerData_FromCustom();

	void UpdateContainerData(UGridInvSys_ContainerGridLayoutWidget* ContainerLayout);

	void PrintDebugOccupiedGrid(const FString& PrintReason = "") const;

protected:
	// 设置容器数据的方式，若库存对象设置了装备片段则该值通常设置为 FromEquip，反之设置为 FromWidget（需要控件片段） 或 Custom
	UPROPERTY(EditDefaultsOnly, Category = "Inventory Fragment")
	EGridInvSys_ContainerLayoutDataType ContainerLayoutDataType;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory Fragment",
		meta = (EditCondition = "ContainerLayoutDataType == EGridInvSys_ContainerLayoutDataType::Custom"))
	TArray<FIntPoint> CustomContainerLayoutData;
	
private:
	// [Server] 记录所有被占据的网格，第一层为 GridID
	// 第二层为不同格子索引 【0、1、2、3】【4、5、6、7】...
	TArray<TArray<bool>> OccupiedGrid;

	// [Server]
	TArray<FIntPoint> ContainerGridSize; // 容器内各个网格的宽度

	FGameplayMessageListenerHandle OnEquipItemInstanceHandle;
	FGameplayMessageListenerHandle OnUnEquipItemInstanceHandle;

	FGameplayMessageListenerHandle ItemPositionChangeHandle;
};
