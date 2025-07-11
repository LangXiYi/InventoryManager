// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InventoryObject/GridInvSys_GridEquipContainerObject.h"
#include "Data/GridInvSys_ItemFragment_ItemType.h"
#include "Widgets/InvSys_EquipContainerSlotWidget.h"
#include "GridInvSys_EquipContainerSlotWidget.generated.h"

struct FGridInvSys_ItemPosition;
class UInvSys_InventoryItemInstance;
class UGridInvSys_ContainerGridItemWidget;

/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_EquipContainerSlotWidget : public UInvSys_EquipContainerSlotWidget
{
	GENERATED_BODY()

public:
	/*UGridInvSys_ContainerGridItemWidget* FindGridItemWidget(const FGridInvSys_ItemPosition& ItemPosition) const;

	UGridInvSys_ContainerGridItemWidget* FindGridItemWidget(const UInvSys_InventoryItemInstance* NewItemInstance) const;*/

	void SetEquipItemType(EGridInvSys_InventoryItemType EquipmentSupportType);

protected:
	virtual bool CheckIsCanDrop_Implementation(UInvSys_InventoryItemInstance* InItemInstance) override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Equip Slot")
	EGridInvSys_InventoryItemType EquipItemType;
};
