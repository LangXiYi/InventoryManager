// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InvSys_InventoryWidget.h"
#include "InvSys_EquipSlotWidget.generated.h"

class UInvSys_InventoryItemInstance;
class UInvSys_BaseInventoryObject;
class UInvSys_BaseEquipmentObject;
/**
 * 
 */
UCLASS()
class BASEINVENTORYSYSTEM_API UInvSys_EquipSlotWidget : public UInvSys_InventoryWidget
{
	GENERATED_BODY()

public:
	virtual void UpdateEquipItem(UInvSys_InventoryItemInstance* NewItemInstance);
	
	void SetInventoryObject(UInvSys_BaseEquipmentObject* NewInvObj);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnUnEquipInventoryItem();
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnEquipInventoryItem(UInvSys_InventoryItemInstance* NewItemInstance);
	
protected:
	UPROPERTY()
	TObjectPtr<UInvSys_BaseInventoryObject> InvObj;

	UPROPERTY()
	TObjectPtr<UInvSys_InventoryItemInstance> ItemInstance;
};
