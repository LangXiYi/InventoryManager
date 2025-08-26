// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/NamedSlot.h"
#include "InvSys_InventoryWidgetSlot.generated.h"

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryWidgetSlot : public UNamedSlot
{
	GENERATED_BODY()

public:
	FGameplayTag GetInventoryTag() const { return InventoryTag; }

	void AddInventorySlotChild(UWidget* Widget);

	template<class T = UUserWidget>
	T* GetInventorySlotChild() const;
	
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag Slot")
	FGameplayTag InventoryTag;
};

template <class T>
T* UInvSys_InventoryWidgetSlot::GetInventorySlotChild() const
{
	return (T*)GetChildAt(0);
}
