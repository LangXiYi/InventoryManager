// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/CanvasPanel.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridWidget.generated.h"

class UInvSys_InventoryItemInstance;
class UGridInvSys_ContainerGridItemWidget;

UENUM()
enum class EGridInvSys_DropType : uint8
{
	None = 0, // false 无法放置
	FreeSpace,
	SizeEqual,
	SizeGreaterThan_ComponentNotEqual,
	SizeGreaterThan_ComponentEqual,
	SizeLessThan
};

UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

	friend class UGridInvSys_ContainerGridLayoutWidget;

public:
	void RemoveAllInventoryItem();

	bool HasEnoughFreeSpace(FIntPoint IntPoint, FIntPoint ItemSize, const TArray<UWidget*>& Ignores = {}) const;

	void FindContainerGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray, FIntPoint Position,
		FIntPoint ItemSize, const TSet<UGridInvSys_ContainerGridItemWidget*>& Ignores = {});

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void ResetDragDropData();

	/** 放下物品至容器的目标位置，From必须是来自容器对象而非装备槽，仅在同容器组件下有效！！ */
	bool TryDropItemFromContainer(UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& DropPosition);

	void ShowDragGridEffect(FIntPoint Position, FIntPoint Size, bool bIsRight);
	
public:
	/**
	 * Getter Or Setter
	 **/

	int32 GetItemIndex(const FIntPoint Position) const;
	UGridInvSys_ContainerGridItemWidget* GetGridItemWidget(FIntPoint Position) const;

	bool FindValidPosition(FIntPoint ItemSize, FIntPoint& OutPosition, const TArray<UWidget*>& Ignores = {}) const;

	/**
	 * 根据鼠标所在的屏幕位置为中点，计算目标左上角的网格坐标
	 */
	FIntPoint CalculateGridOriginPoint(const FVector2D LocalPosition, const FIntPoint ItemSize, FVector2D OffsetRate = FVector2D(0, 0)) const;

	TArray<UGridInvSys_ContainerGridItemWidget*> GetAllContainerGridItems() const;

	template<class T = UWidget>
	void GetContainerGridItems(TArray<T*>& OutArray, FIntPoint Position, FIntPoint Size, const TArray<UWidget*>& Ignores = {}) const
	{
		OutArray.Empty();
		OutArray.Reserve(Size.X * Size.Y);
		TArray<int32> OutIndexes;
		GetContainerGridItemIndexes(OutIndexes, Position, Size);
		for (const int32 Index : OutIndexes)
		{
			T* GridItemWidget = Cast<T>(ContainerPanel->GetChildAt(Index));
			if (Ignores.Contains(GridItemWidget))
			{
				continue;
			}
			OutArray.AddUnique(GridItemWidget);
		}
	}

	/**
	 * 获取区域下所有容器网格项的索引
	 * @param OutArray 输出索引
	 * @param Position 目标位置
	 * @param Size 物品大小
	 */
	void GetContainerGridItemIndexes(TArray<int32>& OutArray, const FIntPoint Position, const FIntPoint Size) const;

	void GetOccupiedGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray,
		FIntPoint Position, FIntPoint Size, const TArray<UWidget*>& Ignores = {}) const;

	void GetAllHoveredItemInstances(TArray<class UGridInvSys_InventoryItemInstance*>& OutArray, FIntPoint Position, FIntPoint Size) const;

	void FindAllFreeGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray, const TArray<UWidget*>& Ignores = {}) const;
	
	FORCEINLINE int32 GetContainerGridID() const { return ContainerGridID; }

	FORCEINLINE FIntPoint GetContainerGridSize() const { return ContainerGridSize; }

	bool IsValidPosition(const FIntPoint Position) const;

	bool IsInRangePosition(FIntPoint TargetPos, FIntPoint OriginPos, FIntPoint Size) const;

	bool IsInRange(FIntPoint TargetPos, FIntPoint TargetSize, FIntPoint OriginPos, FIntPoint Size) const;

	bool IsInContainer(FIntPoint TargetPos, FIntPoint TargetSize) const;

protected:
	void TryDropItemInstance_FreeSpace(UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& DropPosition);

	void TryDropItemInstance_SizeEqual(UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& DropPosition);

	void TryDropItemInstance_SizeGreaterThan_ComponentNotEqual(UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& DropPosition);

	void TryDropItemInstance_SizeGreaterThan_ComponentEqual(UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& DropPosition);

	void TryDropItemInstance_SizeLessThan(UInvSys_InventoryItemInstance* ItemInstance, const FGridInvSys_ItemPosition& DropPosition);

	// 判断目标位置能否放置物品，注意：使用此方法时From必须是来自其他容器
	EGridInvSys_DropType IsCanDropItemFromContainer(UInvSys_InventoryItemInstance* ItemInstance,
		FIntPoint ToPosition, EGridInvSys_ItemDirection ItemDirection) const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Container Grid", meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> ContainerPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Container Grid", meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	TSubclassOf<UGridInvSys_ContainerGridItemWidget> GridItemWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	FIntPoint ContainerGridSize = FIntPoint(1, 1);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	int32 ContainerGridID;

private:
	UPROPERTY(Transient)
	TArray<UGridInvSys_ContainerGridItemWidget*> LastDropOverItems;

	// 上一次拖拽时，计算得到的坐标位置
	FIntPoint LastDropOriginPosition;
};
