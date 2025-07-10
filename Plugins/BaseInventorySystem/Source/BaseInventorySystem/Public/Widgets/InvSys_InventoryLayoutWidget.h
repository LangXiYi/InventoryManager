// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InvSys_InventoryWidget.h"
#include "InvSys_InventoryLayoutWidget.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryLayoutWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	UInvSys_TagSlot* FindTagSlot(FGameplayTag InSlotTag);

	void CollectAllTagSlots();
	
protected:
	UPROPERTY(Transient)
	TMap<FGameplayTag, UInvSys_TagSlot*> TagSlots;
};
