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
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridItemWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	void OnConstructGridItem(UGridInvSys_ContainerGridWidget* InContainerGrid, FIntPoint InPosition);
	
	void SetContainerGridWidget(UGridInvSys_ContainerGridWidget* InContainerGridWidget);

	virtual void UpdateItemInfo(const FGridInvSys_InventoryItem&);

	virtual void RemoveItemInfo();

	void UpdateItemInstance(UInvSys_InventoryItemInstance* NewItemInstance);

	void RemoveItemInstance();
	

	FORCEINLINE UGridInvSys_ContainerGridWidget* GetContainerGridWidget() const;

	FORCEINLINE UInvSys_InventoryItemInfo* GetItemInfo() const;

	template<class T>
	T* GetItemInfo() const
	{
		return nullptr;
	}

	FORCEINLINE FIntPoint GetPosition() const;
	
	FORCEINLINE FIntPoint GetOriginPosition() const;

	FGridInvSys_InventoryItemPosition GetGridItemPosition()const
	{
		return FGridInvSys_InventoryItemPosition();
	}
	
	FORCEINLINE UGridInvSys_ContainerGridItemWidget* GetOriginGridItemWidget() const;

	FORCEINLINE bool IsOccupied() const { return bIsOccupied; }

	FORCEINLINE EGridInvSys_ItemDirection GetItemDirection() const;

	/** 获取Item的大小，该大小会收到方向的影响。 */
	FIntPoint GetItemSize() const;

	UFUNCTION(BlueprintPure)
	FORCEINLINE FName GetItemUniqueID() const;
	
	FORCEINLINE FName GetSlotName() const;
	
	TArray<UWidget*> GetOccupiedGridItems();

	FIntPoint CalculateRelativePosition(const UGridInvSys_ContainerGridItemWidget* Parent) const;

	int32 GetGridID() const;

	template<class T>
	T* GetItemInstance() const
	{
		return (T*)ItemInstance;
	}

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnItemInstanceChange(const UInvSys_InventoryItemInstance* NewItemInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRemoveItemInstance();

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
	UInvSys_InventoryItemInstance* ItemInstance;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	bool bIsOccupied;
};
