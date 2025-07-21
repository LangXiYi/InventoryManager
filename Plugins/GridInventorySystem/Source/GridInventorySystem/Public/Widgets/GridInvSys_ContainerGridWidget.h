// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Components/CanvasPanel.h"
#include "Components/GridPanel.h"
#include "Components/UniformGridPanel.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridWidget.generated.h"

class UInvSys_InventoryItemInstance;
class UGridInvSys_ContainerGridLayoutWidget;
class UGridInvSys_DragItemWidget;
class UGridInvSys_ContainerGridDropWidget;
class UGridInvSys_ContainerGridItemWidget;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	void ConstructGridItems(int32 InGridID);

	void UpdateInventoryItem(const FGridInvSys_InventoryItem& InventoryItem);

	void RemoveInventoryItem(const FGridInvSys_InventoryItem& InventoryItem);
	void RemoveAllInventoryItem();

	bool HasEnoughFreeSpace(FIntPoint IntPoint, FIntPoint ItemSize, const TArray<UWidget*>& Ignores = {}) const;

	void FindContainerGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray, FIntPoint Position,
		FIntPoint ItemSize, const TSet<UGridInvSys_ContainerGridItemWidget*>& Ignores = {});

	void UpdateContainerGridSize();

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void ResetDragDropData();

	/** 放下物品至容器的目标位置，From必须是来自容器对象而非装备槽，仅在同容器组件下有效！！ */
	bool TryDropItemFromContainer(UGridInvSys_ContainerGridWidget* FromContainer,
		UInvSys_InventoryItemInstance* ItemInstance, FIntPoint FromItemSize, FGridInvSys_ItemPosition ItemPositionData);

	void ShowDragGridEffect(FIntPoint Position, FIntPoint Size, bool bIsRight);
	
	/**
	 * Getter Or Setter
	 **/

public:
	int32 GetItemIndex(const FIntPoint Position) const;
	UGridInvSys_ContainerGridItemWidget* GetGridItemWidget(FIntPoint Position) const;
	UGridInvSys_ContainerGridDropWidget* GetContainerGridDropItem(FIntPoint Position) const;

	bool FindValidPosition(FIntPoint ItemSize, FIntPoint& OutPosition, const TArray<UWidget*>& Ignores = {}) const;

	/**
	 * 根据鼠标所在的屏幕位置为中点，计算目标左上角的网格坐标
	 * @param LocalPosition 屏幕位置，注意需要转换为本地位置，即相对该控件的位置
	 * @param ItemSize 物品占据的网格大小
	 */
	FIntPoint CalculateGridOriginPoint(const FVector2D LocalPosition, const FIntPoint ItemSize) const;

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

	void FindAllFreeGridItems(TArray<UGridInvSys_ContainerGridItemWidget*>& OutArray, const TArray<UWidget*>& Ignores = {}) const;
	
	FORCEINLINE int32 GetContainerGridID() const;

	FORCEINLINE FName GetSlotName() const;

	FORCEINLINE FIntPoint GetContainerGridSize() const { return ContainerGridSize; }

	bool IsValidPosition(const FIntPoint Position) const;

	bool IsInRangePosition(FIntPoint TargetPos, FIntPoint OriginPos, FIntPoint Size) const;

	bool IsInRange(FIntPoint TargetPos, FIntPoint TargetSize, FIntPoint OriginPos, FIntPoint Size) const;

	bool IsInContainer(FIntPoint TargetPos, FIntPoint TargetSize) const;

protected:
	// 判断目标位置能否放置物品，注意：使用此方法时From必须是来自其他容器
	bool IsCanDropItemFromContainer(UGridInvSys_ContainerGridWidget* FromContainer, FIntPoint ToPosition, FIntPoint FromItemSize) const;

protected:
	// 废弃！！！
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Container Grid", meta = (BindWidget))
	TObjectPtr<class UGridPanel> ContainerGridItemPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Container Grid", meta = (BindWidget))
	TObjectPtr<class UUniformGridPanel> ContainerGridDropPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Container Grid", meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> ContainerPanel;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Container Grid", meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	TSubclassOf<UGridInvSys_ContainerGridItemWidget> GridItemWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	TSubclassOf<UGridInvSys_ContainerGridDropWidget> GridDropWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	TSubclassOf<UGridInvSys_DragItemWidget> DragItemWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	FIntPoint ContainerGridSize = FIntPoint(1, 1);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	int32 ContainerGridID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Container Grid")
	FName SlotName;

private:
	UPROPERTY()
	TArray<UGridInvSys_ContainerGridItemWidget*> LastDropOverItems;

	// 上一次拖拽时，计算得到的坐标位置
	FIntPoint LastDropOriginPosition;
};
