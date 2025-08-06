// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "Widgets/InvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridItemWidget.generated.h"

class UInvSys_InventoryItemInstance;
struct FGridInvSys_InventoryItem;
class UGridPanel;
class UGridInvSys_ContainerGridWidget;
class UGridInvSys_InventoryComponent;
class UGridInvSys_DragDropWidget;
class USizeBox;
class UInvSys_InventoryItemInfo;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void OnConstructGridItem(UGridInvSys_ContainerGridWidget* InContainerGrid, FIntPoint InPosition);
	
	void AddItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);

	void RemoveItemInstance();

	FORCEINLINE UGridInvSys_ContainerGridWidget* GetContainerGridWidget() const;

	template<class T>
	T* GetItemInfo() const
	{
		return nullptr;
	}

	FORCEINLINE FIntPoint GetPosition() const;
	
	FORCEINLINE FIntPoint GetOriginPosition() const;

	FORCEINLINE UGridInvSys_ContainerGridItemWidget* GetOriginGridItemWidget() const;

	bool IsOccupied();

	FORCEINLINE EGridInvSys_ItemDirection GetItemDirection() const;

	UFUNCTION(BlueprintPure)
	FORCEINLINE FName GetItemUniqueID() const;
	
	FORCEINLINE FName GetSlotName() const;
	
	TArray<UWidget*> GetOccupiedGridItems();

	FIntPoint CalculateRelativePosition(const UGridInvSys_ContainerGridItemWidget* Parent) const;

	UFUNCTION(BlueprintPure)
	int32 GetGridID() const;

	template<class T = UInvSys_InventoryItemInstance>
	T* GetItemInstance() const
	{
		return (T*)ItemInstance.Get();
	}

	FIntPoint GetItemSize() const
	{
		return GridItemSize;
	}

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnAddItemInstance(const UInvSys_InventoryItemInstance* NewItemInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRemoveItemInstance();

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "DraggingDisplayWidget")
	void OnDraggingHovered(bool bIsCanDrop);

	UFUNCTION(BlueprintImplementableEvent, Category = "DraggingDisplayWidget")
	void OnEndDraggingHovered();

private:
	/** 获取Item的大小，该大小会收到方向的影响。 */
	FIntPoint CalculateGridItemSize(UInvSys_InventoryItemInstance* InItemInstance) const;

protected:
	/*UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item", meta = (BindWidget))
	TObjectPtr<UGridInvSys_DragDropWidget> DragDropWidget;*/

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item", meta = (BindWidget))
	TObjectPtr<UNamedSlot> ItemSlot;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item", meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox;
	
	UPROPERTY()
	TObjectPtr<UGridInvSys_ContainerGridWidget> ContainerGridWidget;

	UPROPERTY()
	TObjectPtr<UGridInvSys_ContainerGridItemWidget> OriginGridItemWidget;

	/** Item Position Info */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	FIntPoint Position;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	FIntPoint GridItemSize = FIntPoint(1, 1);

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	TWeakObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	bool bIsOccupied;
};
