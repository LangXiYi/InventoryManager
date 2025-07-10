// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/NamedSlot.h"
#include "InvSys_TagSlot.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_TagSlot : public UNamedSlot
{
	GENERATED_BODY()

public:
	FGameplayTag GetSlotTag() const { return SlotTag; }
	
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag Slot")
	FGameplayTag SlotTag;
};
