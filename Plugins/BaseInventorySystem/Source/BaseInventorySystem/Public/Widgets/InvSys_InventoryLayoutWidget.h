// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryWidget.h"
#include "InvSys_InventoryLayoutWidget.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryLayoutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UInvSys_TagSlot* FindTagSlot(FGameplayTag InSlotTag);

	virtual void NativeOnInitialized() override;

	virtual void AddWidget(UUserWidget* Widget, const FGameplayTag& Tag);

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
protected:
	UPROPERTY(Transient)
	TMap<FGameplayTag, UInvSys_TagSlot*> TagSlots;
};
