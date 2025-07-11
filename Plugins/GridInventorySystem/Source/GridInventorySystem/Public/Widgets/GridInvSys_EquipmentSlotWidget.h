// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/GridInvSys_ItemFragment_ItemType.h"
#include "Widgets/InvSys_EquipSlotWidget.h"
#include "GridInvSys_EquipmentSlotWidget.generated.h"

class UInvSys_InventoryItemInstance;
class UGridInvSys_DragDropWidget;
class UGridInvSys_ContainerGridLayoutWidget;
struct FInvSys_InventoryItem;

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_EquipmentSlotWidget : public UInvSys_EquipSlotWidget
{
	GENERATED_BODY()

public:
	void SetEquipItemType(EGridInvSys_InventoryItemType NewEquipItemType)
	{
		EquipItemType = NewEquipItemType;
	}
	
protected:
	virtual bool CheckIsCanDrop_Implementation(UInvSys_InventoryItemInstance* InItemInstance) override;
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGridInvSys_DragDropWidget> DragDropWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Equip Slot")
	EGridInvSys_InventoryItemType EquipItemType;
};
