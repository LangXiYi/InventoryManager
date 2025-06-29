// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridInvSys_InventoryWidget.h"
#include "GridInvSys_ContainerGridItemWidget.generated.h"

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
	void InitContainerGridItem(UGridInvSys_ContainerGridWidget* InContainerGridWidget, FIntPoint InPosition);

	virtual void UpdateItemInfo(UInvSys_InventoryItemInfo* NewItemInfo);

	virtual void RemoveItemInfo();

	FORCEINLINE UGridInvSys_ContainerGridWidget* GetContainerGridWidget() const;

	FORCEINLINE UObject* GetItemInfo() const;

	FORCEINLINE FIntPoint GetPosition() const;

	FORCEINLINE bool IsOccupied() const;

	FORCEINLINE void SetSlotName(FName NewSlotName);

	FORCEINLINE void SetGridID(FName NewGridID);

	/**
	 * 计算当前网格单元相对另一个网格单元的位置
	 */
	FIntPoint CalculateRelativePosition(UGridInvSys_ContainerGridItemWidget* Parent) const;
	
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
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	FIntPoint Position;
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	bool bIsOccupied = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	TObjectPtr<UInvSys_InventoryItemInfo> ItemInfo;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid Item")
	TObjectPtr<UGridInvSys_InventoryComponent> InventoryComponent;

};
