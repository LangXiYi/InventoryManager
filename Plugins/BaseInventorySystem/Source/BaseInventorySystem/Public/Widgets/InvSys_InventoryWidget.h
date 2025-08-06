// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/InventoryObject/InvSys_BaseInventoryObject.h"
#include "InvSys_InventoryWidget.generated.h"

class UInvSys_BaseInventoryObject;
class UInvSys_TagSlot;
class UInvSys_InventoryComponent;

/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_InventoryWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UInvSys_InventoryFragment_DisplayWidget;

public:
	virtual void RefreshInventoryWidget(UInvSys_BaseInventoryObject* NewInventoryObject);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRefreshWidget();
	
	virtual void NativeConstruct() override;
	
	virtual void SetInventoryComponent(UInvSys_InventoryComponent* NewInvComp);

	UInvSys_InventoryComponent* GetInventoryComponent() const;

	/*UFUNCTION(BlueprintImplementableEvent)
	UInvSys_InventoryComponent* GetPlayerInventoryComponent() const;*/

	template<class T>
	T* GetInventoryComponent() const
	{
		check(InventoryObject)
		return Cast<T>(InventoryObject->GetInventoryComponent());		
	}

	FORCEINLINE void SetSlotTag(FGameplayTag InSlotTag)
	{
		SlotTag = InSlotTag;
	}

	FORCEINLINE FGameplayTag GetSlotTag() const
	{
		check(InventoryObject)
		return InventoryObject->GetInventoryObjectTag();
	}

	void SetInventoryObject(UInvSys_BaseInventoryObject* NewInventoryObject);

	UInvSys_BaseInventoryObject* GetInventoryObject()
	{
		check(InventoryObject);
		return InventoryObject;
	}

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Container Grid Layout", meta = (ExposeOnSpawn))
	TObjectPtr<UInvSys_BaseInventoryObject> InventoryObject;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Widget")
	TWeakObjectPtr<UInvSys_InventoryComponent> InventoryComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Container Grid Layout")
	FGameplayTag SlotTag;
};
