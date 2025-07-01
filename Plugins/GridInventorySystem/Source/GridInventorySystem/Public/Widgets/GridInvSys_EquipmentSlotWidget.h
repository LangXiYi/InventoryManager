// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/GridInvSys_InventoryWidget.h"
#include "GridInvSys_EquipmentSlotWidget.generated.h"

class UGridInvSys_DragDropWidget;
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
	virtual void UpdateOccupant(const FInvSys_InventoryItem& NewOccupant);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateOccupant(const FInvSys_InventoryItem& NewOccupant);

public:
	virtual void SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp) override;
	
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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGridInvSys_DragDropWidget> DragDropWidget;
};
