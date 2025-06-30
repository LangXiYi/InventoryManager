// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/GridInvSys_InventoryWidget.h"
#include "GridInvSys_EquipmentSlotWidget.generated.h"

class UGridInvSys_ContainerGridLayoutWidget;
struct FInvSys_InventoryItem;
/**
 * 
 */
UCLASS()
class GRIDINVENTORYSYSTEM_API UGridInvSys_EquipmentSlotWidget : public UGridInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void UpdateOccupant(const FInvSys_InventoryItem& NewOccupant);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateOccupant(const FInvSys_InventoryItem& NewOccupant);

public:	
	void SetSlotName(FName NewSlotName)
	{
		SlotName = NewSlotName;
	}

	FName GetSlotName() const
	{
		return SlotName;
	}

protected:
	FName SlotName;
};
