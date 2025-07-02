// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_CommonType.h"
#include "GridInvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridItemWidget.generated.h"

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
class GRIDINVENTORYSYSTEM_API UGridInvSys_ContainerGridItemWidget : public UGridInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	void SetContainerGridWidget(UGridInvSys_ContainerGridWidget* InContainerGridWidget);

	virtual void UpdateItemInfo(const FGridInvSys_InventoryItem&);

	virtual void RemoveItemInfo();

	FORCEINLINE UGridInvSys_ContainerGridWidget* GetContainerGridWidget() const;

	FORCEINLINE UInvSys_InventoryItemInfo* GetItemInfo() const;

	template<class T>
	T* GetItemInfo() const
	{
		return Cast<T>(InventoryItem.BaseItemData.ItemInfo);
	}

	FORCEINLINE FIntPoint GetPosition() const;
	
	FORCEINLINE FIntPoint GetOriginPosition() const;

	const FGridInvSys_InventoryItemPosition& GetGridItemPosition()const
	{
		return InventoryItem.ItemPosition;
	}
	
	FORCEINLINE UGridInvSys_ContainerGridItemWidget* GetOriginGridItemWidget() const;

	FORCEINLINE bool IsOccupied() const;

	FORCEINLINE void SetSlotName(FName NewSlotName);

	FORCEINLINE void SetGridID(FName NewGridID);

	FORCEINLINE EGridInvSys_ItemDirection GetItemDirection() const;

	FIntPoint GetItemSize() const;

	FORCEINLINE FName GetItemUniqueID() const;
	FORCEINLINE FName GetSlotName() const;

	const FGridInvSys_InventoryItem& GetInventoryItemData() const
	{
		return InventoryItem;
	}
	
	TArray<UWidget*> GetOccupiedGridItems();

	FIntPoint CalculateRelativePosition(const UGridInvSys_ContainerGridItemWidget* Parent) const;

	FName GetGridID() const;

protected:
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnItemInfoChanged(UInvSys_InventoryItemInfo* NewOccupant);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item", meta = (BindWidget))
	TObjectPtr<UGridInvSys_DragDropWidget> DragDropWidget;

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
	FName SlotName;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	FName GridID;
	//UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	//FIntPoint Position;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	bool bIsOccupied = false;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	FGridInvSys_InventoryItem InventoryItem;
};
