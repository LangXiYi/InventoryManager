// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryWidget.generated.h"

class UInvSys_TagSlot;
class UInvSys_InventoryComponent;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp);

	UInvSys_InventoryComponent* GetInventoryComponent() const;

	/*UFUNCTION(BlueprintImplementableEvent)
	UInvSys_InventoryComponent* GetPlayerInventoryComponent() const;*/

	template<class T>
	T* GetInventoryComponent() const
	{
		return Cast<T>(InventoryComponent);		
	}

	FORCEINLINE void SetSlotTag(FGameplayTag InSlotTag)
	{
		SlotTag = InSlotTag;
	}

	FORCEINLINE FGameplayTag GetSlotTag() const
	{
		return SlotTag;
	}
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory Widget", meta = (ExposeOnSpawn))
	TWeakObjectPtr<UInvSys_InventoryComponent> InventoryComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Container Grid Layout", meta = (ExposeOnSpawn))
	FGameplayTag SlotTag;
};
