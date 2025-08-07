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

	bool HasEnoughFreeSpace(FIntPoint ToPosition, int32 ToGridID, FIntPoint ItemSize);

	void UpdateContainerGridItemState(UGridInvSys_InventoryItemInstance* GridItemInstance, const FGridInvSys_ItemPosition& ItemPosition, bool IsOccupy);

	bool FindEmptyPosition(FIntPoint ItemSize, FGridInvSys_ItemPosition& OutPosition);

	virtual void RemoveAllItemInstance() override;

protected:
	virtual void NativeOnContainerEntryAdded(FInvSys_InventoryItemChangedMessage ChangeInfo) override;
	virtual void NativeOnContainerEntryRemove(FInvSys_InventoryItemChangedMessage ChangeInfo) override;
	
private:
	void UpdateContainerData_FromEquip();

	void UpdateContainerData_FromWidget();
	
	void UpdateContainerData_FromCustom();

	void UpdateContainerData(UGridInvSys_ContainerGridLayoutWidget* ContainerLayout);

	void PrintDebugOccupiedGrid(const FString& PrintReason = "");

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
